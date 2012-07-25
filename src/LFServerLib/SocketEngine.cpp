#include "SocketEngine.h"
#include "LogEngine.h"
#include "BigNumber.h"

//////////////////////////////////////////////////////////////////////////
//宏定义
#define INDEX_ALL_SOCKET			0xFFFF								//所有连接

#define TIME_BREAK_READY			9000L								//中断时间
#define TIME_BREAK_CONNECT			4000L								//中断时间
#define TIME_DETECT_SOCKET			20000L								//监测时间

//////////////////////////////////////////////////////////////////////////

//动作定义
#define QUEUE_SEND_REQUEST			1									//发送标识
#define QUEUE_SAFE_CLOSE			2									//安全关闭
#define QUEUE_DETECT_SOCKET			3									//检测连接

//其他定义
#define GetIndexFromeSocketID(a)	LOWORD(a)
#define GetRoundIDFromSocketID(a)	HIWORD(a)

//发送请求结构
struct tagSendDataRequest
{
	DWORD							dwCommand;							//命令
	WORD							wIndex;								//连接索引
	WORD							wRountID;							//循环索引
	WORD							wDataSize;							//数据大小
	BYTE							cbSendBuf[SOCKET_BUFFER];			//发送缓冲
};

//安全关闭
struct tagSafeCloseSocket
{
	WORD							wIndex;								//连接索引
	WORD							wRountID;							//循环索引
};

//////////////////////////////////////////////////////////////////////////

//构造函数
COverLapped::COverLapped(enOperationType OperationType) : m_OperationType(OperationType)
{
	memset(&m_WSABuffer,0,sizeof(m_WSABuffer));
	memset(&m_OverLapped,0,sizeof(m_OverLapped));
}

//析构函数
COverLapped::~COverLapped()
{
}

//////////////////////////////////////////////////////////////////////////

//构造函数
COverLappedSend::COverLappedSend() : COverLapped(OperationType_Send)
{
	m_WSABuffer.len=0;
	m_WSABuffer.buf=(char *)m_cbBuffer;
}

//析构函数
COverLappedSend::~COverLappedSend()
{
}

//////////////////////////////////////////////////////////////////////////
//构造函数
CServerSocketItem::CServerSocketItem(WORD wIndex, CServerSocketItemSink * pIServerSocketItemSink) 
: m_wIndex(wIndex), m_pServerSocketItemSink(pIServerSocketItemSink)
{
	m_wRountID=0;
	m_wRecvSize=0;
	m_bNotify=true;
	m_bRecvIng=false;
	m_bCloseIng=false;
	m_dwClientAddr=0;
	m_dwConnectTime=0;
	m_dwSendTickCount=0;
	m_dwRecvTickCount=0;
	m_dwSendPacketCount=0;
	m_dwRecvPacketCount=0;
	m_hSocket=INVALID_SOCKET;
}

//析构函数
CServerSocketItem::~CServerSocketItem(void)
{
	//删除空闲重叠 IO
	for(COverLappedSendSet::iterator iter = m_OverLappedSendFree.begin();
		iter != m_OverLappedSendFree.end(); ++iter)
	{
		delete *iter;
	}

	//删除活动重叠 IO
	for(COverLappedSendSet::iterator iter = m_OverLappedSendActive.begin();
		iter != m_OverLappedSendActive.end(); ++iter)
	{
		delete *iter;
	}
}

//获取发送结构
COverLappedSend * CServerSocketItem::GetSendOverLapped()
{
	//寻找空闲结构
	COverLappedSend * pOverLappedSend=NULL;
	int nFreeCount=m_OverLappedSendFree.size();
	if (nFreeCount>0)
	{
		pOverLappedSend=*m_OverLappedSendFree.begin();
		ASSERT(pOverLappedSend!=NULL);
		m_OverLappedSendFree.erase(m_OverLappedSendFree.begin());
		m_OverLappedSendActive.insert(pOverLappedSend);
		return pOverLappedSend;
	}

	//新建发送结构
	try
	{
		pOverLappedSend=new COverLappedSend;
		ASSERT(pOverLappedSend!=NULL);
		m_OverLappedSendActive.insert(pOverLappedSend);
		return pOverLappedSend;
	}
	catch (...) { }
	return NULL;
}

//绑定对象
DWORD CServerSocketItem::Attach(SOCKET hSocket, DWORD dwClientAddr)
{
	//效验数据
	ASSERT(dwClientAddr!=0);
	ASSERT(m_bRecvIng==false);
	ASSERT(IsValidSocket()==false);
	ASSERT(hSocket!=INVALID_SOCKET);

	//设置变量
	m_bNotify=false;
	m_bRecvIng=false;
	m_bCloseIng=false;
	m_hSocket=hSocket;
	m_dwClientAddr=dwClientAddr;
	m_dwRecvTickCount=GetTickCount();
	m_dwConnectTime=(DWORD)time(NULL);

	BigNumber bn;
	bn.SetHexStr(COMMAND_AUTHCRYPT_KEY);
	m_AuthServerCrypt.Init(&bn);

	//发送通知
	m_pServerSocketItemSink->OnSocketAcceptEvent(this);

	return GetSocketID();
}

//发送函数
bool CServerSocketItem::SendData(DWORD dwCommand, void * pData, WORD wDataSize, WORD wRountID)
{
	//效验参数
	ASSERT(wDataSize<=SOCKET_BUFFER);

	//效验状态
	if (m_bCloseIng==true) return false;
	if (m_wRountID!=wRountID) return false;
	if (m_dwRecvPacketCount==0) return false;
	if (IsValidSocket()==false) return false;
	if (wDataSize>SOCKET_BUFFER) return false;

	//寻找发送结构
	COverLappedSend * pOverLappedSend=GetSendOverLapped();
	ASSERT(pOverLappedSend!=NULL);
	if (pOverLappedSend==NULL) return false;

	//构造数据
	CMD_Head * pHead=(CMD_Head *)pOverLappedSend->m_cbBuffer;
	pHead->dwCommand=dwCommand;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		memcpy(pHead+1,pData,wDataSize);
	}
	WORD wSendSize=EncryptBuffer(pOverLappedSend->m_cbBuffer,sizeof(CMD_Head)+wDataSize,sizeof(pOverLappedSend->m_cbBuffer));
	pOverLappedSend->m_WSABuffer.len=wSendSize;

	//发送数据
	if (m_OverLappedSendActive.size()==1)
	{
		DWORD dwThancferred=0;
		int iRetCode=WSASend(m_hSocket,&pOverLappedSend->m_WSABuffer,1,&dwThancferred,0,&pOverLappedSend->m_OverLapped,NULL);
		if ((iRetCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
		{
			OnSendCompleted(pOverLappedSend,0L);
			return false;
		}
	}

	return true;
}

//投递接收
bool CServerSocketItem::RecvData()
{
	//效验变量
	ASSERT(m_bRecvIng==false);
	ASSERT(m_hSocket!=INVALID_SOCKET);

	//判断关闭
	if (m_bCloseIng==true)
	{
		if (m_OverLappedSendActive.size()==0) CloseSocket(m_wRountID);
		return false;
	}

	//接收数据
	m_bRecvIng=true;
	DWORD dwThancferred=0,dwFlags=0;
	int iRetCode=WSARecv(m_hSocket,&m_OverLappedRecv.m_WSABuffer,1,&dwThancferred,&dwFlags,&m_OverLappedRecv.m_OverLapped,NULL);
	if ((iRetCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
	{
		m_bRecvIng=false;
		CloseSocket(m_wRountID);
		return false;
	}

	return true;
}

//关闭连接
bool CServerSocketItem::CloseSocket(WORD wRountID)
{
	//状态判断
	if (m_wRountID!=wRountID) return false;

	//关闭连接
	if (m_hSocket!=INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket=INVALID_SOCKET;
	}

	//判断关闭
	if ((m_bRecvIng==false)&&(m_OverLappedSendActive.size()==0)) OnCloseCompleted();

	return true;
}

//设置关闭
bool CServerSocketItem::ShutDownSocket(WORD wRountID)
{
	//状态判断
	if (m_wRountID!=wRountID) return false;
	if (m_hSocket==INVALID_SOCKET) return false;

	//设置变量
	if (m_bCloseIng==false)
	{
		m_bCloseIng=true;
		if (m_OverLappedSendActive.size()==0) CloseSocket(wRountID);
	}

	return true;
}

//重置变量
void CServerSocketItem::ResetSocketData()
{
	//效验状态
	ASSERT(m_hSocket==INVALID_SOCKET);

	//重置数据
	m_wRountID++;
	m_wRecvSize=0;
	m_dwClientAddr=0;
	m_dwConnectTime=0;
	m_dwSendTickCount=0;
	m_dwRecvTickCount=0;
	m_dwSendPacketCount=0;
	m_dwRecvPacketCount=0;

	//状态变量
	m_bNotify=true;
	m_bRecvIng=false;
	m_bCloseIng=false;
	m_OverLappedSendFree.insert(m_OverLappedSendActive.begin(), m_OverLappedSendActive.end());
	m_OverLappedSendActive.clear();

	return;
}

//发送完成函数
bool CServerSocketItem::OnSendCompleted(COverLappedSend * pOverLappedSend, DWORD dwThancferred)
{
	//效验变量
	ASSERT(pOverLappedSend!=NULL);
	ASSERT(m_OverLappedSendActive.size()>0);
	ASSERT(pOverLappedSend==*m_OverLappedSendActive.begin());

	//释放发送结构
	m_OverLappedSendActive.erase(m_OverLappedSendActive.begin());
	m_OverLappedSendFree.insert(pOverLappedSend);

	//设置变量
	if (dwThancferred!=0) m_dwSendTickCount=GetTickCount();

	//判断关闭
	if (m_hSocket==INVALID_SOCKET)
	{
		m_OverLappedSendFree.insert(m_OverLappedSendActive.begin(), m_OverLappedSendActive.end());
		m_OverLappedSendActive.clear();
		CloseSocket(m_wRountID);
		return true;
	}

	//继续发送数据
	if (m_OverLappedSendActive.size()>0)
	{
		DWORD dwThancferred=0;
		pOverLappedSend=*m_OverLappedSendActive.begin();
		ASSERT(pOverLappedSend!=NULL);
		int iRetCode=WSASend(m_hSocket,&pOverLappedSend->m_WSABuffer,1,&dwThancferred,0,&pOverLappedSend->m_OverLapped,NULL);
		if ((iRetCode==SOCKET_ERROR)&&(WSAGetLastError()!=WSA_IO_PENDING))
		{
			m_OverLappedSendFree.insert(m_OverLappedSendActive.begin(), m_OverLappedSendActive.end());
			m_OverLappedSendActive.clear();
			CloseSocket(m_wRountID);
			return false;
		}
		return true;
	}

	//判断关闭
	if (m_bCloseIng==true) 
		CloseSocket(m_wRountID);

	return true;
}

//接收完成函数
bool CServerSocketItem::OnRecvCompleted(COverLappedRecv * pOverLappedRecv, DWORD dwThancferred)
{
	//效验数据
	ASSERT(m_bRecvIng==true);

	//设置变量
	m_bRecvIng=false;
	m_dwRecvTickCount=GetTickCount();

	//判断关闭
	if (m_hSocket==INVALID_SOCKET)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	//接收数据
	int iRetCode=recv(m_hSocket,(char *)m_cbRecvBuf+m_wRecvSize,sizeof(m_cbRecvBuf)-m_wRecvSize,0);
	if (iRetCode<=0)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	//接收完成
	m_wRecvSize+=iRetCode;
	BYTE cbBuffer[SOCKET_BUFFER];
	CMD_Head * pHead=(CMD_Head *)m_cbRecvBuf;

	//处理数据
	try
	{
		while (m_wRecvSize>=sizeof(CMD_Head))
		{
			//效验数据
			WORD wPacketSize=pHead->CmdInfo.wDataSize;
			if (wPacketSize>SOCKET_BUFFER) throw TEXT("数据包超长");
			if (wPacketSize<sizeof(CMD_Head)) throw TEXT("数据包非法");
			if (pHead->CmdInfo.cbMessageVer!=SOCKET_VER) throw TEXT("数据包版本错误");
			if (m_wRecvSize<wPacketSize) break;

			//提取数据
			CopyMemory(cbBuffer,m_cbRecvBuf,wPacketSize);
			WORD wRealySize=CrevasseBuffer(cbBuffer,wPacketSize);
			ASSERT(wRealySize>=sizeof(CMD_Head));
			m_dwRecvPacketCount++;

			//解释数据
			WORD wDataSize=wRealySize-sizeof(CMD_Head);
			void * pDataBuffer=cbBuffer+sizeof(CMD_Head);
			DWORD dwCommand=((CMD_Head *)cbBuffer)->dwCommand;

			//网络检测
			if (dwCommand!=CMD_DETECT_SOCKET)
			{
				//消息处理
				m_pServerSocketItemSink->OnSocketReadEvent(dwCommand,pDataBuffer,wDataSize,this);			
			}

			//删除缓存数据
			m_wRecvSize-=wPacketSize;
			MoveMemory(m_cbRecvBuf,m_cbRecvBuf+wPacketSize,m_wRecvSize);
		}
	}
	catch (...)
	{ 
		CloseSocket(m_wRountID);
		return false;
	}

	return RecvData();
}

//关闭完成通知
bool CServerSocketItem::OnCloseCompleted()
{
	//效验状态
	ASSERT(m_hSocket==INVALID_SOCKET);
	ASSERT(m_OverLappedSendActive.size()==0);

	//关闭事件
	ASSERT(m_bNotify==false);
	if (m_bNotify==false)
	{
		m_bNotify=true;
		m_pServerSocketItemSink->OnSocketCloseEvent(this);
	}

	//状态变量
	ResetSocketData();

	return true;
}

//加密数据
WORD CServerSocketItem::EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize)
{
	WORD i = 0;
	//效验参数
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(wDataSize<=(sizeof(CMD_Head)+SOCKET_PACKET));
	ASSERT(wBufferSize>=(wDataSize+2*sizeof(DWORD)));

	//效验码与字节映射
	BYTE cbCheckCode=0;
	for (i=sizeof(CMD_Info);i<wDataSize;i++) 
	{
		cbCheckCode+=pcbDataBuffer[i];
	}

	//填写信息头
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	pHead->CmdInfo.cbCheckCode=~cbCheckCode+1;
	pHead->CmdInfo.wDataSize=wDataSize;
	pHead->CmdInfo.cbMessageVer=SOCKET_VER;

	//加密数据-只加密Command
	m_AuthServerCrypt.EncryptSend(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//设置变量
	m_dwSendPacketCount++;

	return wDataSize;
}

//解密数据
WORD CServerSocketItem::CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize)
{
	//效验参数
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(((CMD_Head *)pcbDataBuffer)->CmdInfo.wDataSize==wDataSize);

	//解密数据
	m_AuthServerCrypt.DecryptRecv(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//效验码
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	BYTE cbCheckCode=pHead->CmdInfo.cbCheckCode;;
	for (int i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
	}
	if (cbCheckCode!=0) throw TEXT("数据包效验码错误");

	return wDataSize;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CServerSocketRSThread::CServerSocketRSThread(void)
{
	m_hCompletionPort=NULL;
}

//析构函数
CServerSocketRSThread::~CServerSocketRSThread(void)
{
}

//配置函数
bool CServerSocketRSThread::InitThread(HANDLE hCompletionPort)
{
	ASSERT(hCompletionPort!=NULL);
	m_hCompletionPort=hCompletionPort;
	return true;
}

//运行函数
bool CServerSocketRSThread::RepetitionRun()
{
	//效验参数
	ASSERT(m_hCompletionPort!=NULL);

	//变量定义
	DWORD dwThancferred=0;					
	OVERLAPPED * pOverLapped=NULL;
	COverLapped * pSocketLapped=NULL;
	CServerSocketItem * pServerSocketItem=NULL;

	//等待完成端口
	BOOL bSuccess=GetQueuedCompletionStatus(m_hCompletionPort,&dwThancferred,(PULONG_PTR)&pServerSocketItem,&pOverLapped,INFINITE);
	if ((bSuccess==FALSE)&&(GetLastError()!=ERROR_NETNAME_DELETED)) return false;
	if ((pServerSocketItem==NULL)&&(pOverLapped==NULL)) return false;

	//处理操作
	ASSERT(pOverLapped!=NULL);
	ASSERT(pServerSocketItem!=NULL);
	pSocketLapped=CONTAINING_RECORD(pOverLapped,COverLapped,m_OverLapped);
	switch (pSocketLapped->GetOperationType())
	{
	case OperationType_Recv:	//SOCKET 数据读取
		{
			COverLappedRecv * pOverLappedRecv=(COverLappedRecv *)pSocketLapped;
			CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
			pServerSocketItem->OnRecvCompleted(pOverLappedRecv,dwThancferred);
			break;
		}
	case OperationType_Send:	//SOCKET 数据发送
		{
			COverLappedSend * pOverLappedSend=(COverLappedSend *)pSocketLapped;
			CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
			pServerSocketItem->OnSendCompleted(pOverLappedSend,dwThancferred);
			break;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CSocketAcceptThread::CSocketAcceptThread(void)
{
	m_hCompletionPort=NULL;
	m_pTCPSocketManager=NULL;
	m_hListenSocket=INVALID_SOCKET;
}

//析构函数
CSocketAcceptThread::~CSocketAcceptThread(void)
{
}

//配置函数
bool CSocketAcceptThread::InitThread(HANDLE hCompletionPort, SOCKET hListenSocket, CTCPSocketEngine * pTCPSocketManager)
{
	ASSERT(hCompletionPort!=NULL);
	ASSERT(pTCPSocketManager!=NULL);
	ASSERT(hListenSocket!=INVALID_SOCKET);
	m_hListenSocket=hListenSocket;
	m_hCompletionPort=hCompletionPort;
	m_pTCPSocketManager=pTCPSocketManager;
	return true;
}

//运行函数
bool CSocketAcceptThread::RepetitionRun()
{
	//效验参数
	ASSERT(m_hCompletionPort!=NULL);
	ASSERT(m_pTCPSocketManager!=NULL);

	//设置变量
	SOCKADDR_IN	SocketAddr;
	CServerSocketItem * pServerSocketItem=NULL;
	SOCKET hConnectSocket=INVALID_SOCKET;
	int nBufferSize=sizeof(SocketAddr);

	try
	{
		//监听连接
		hConnectSocket=WSAAccept(m_hListenSocket,(SOCKADDR *)&SocketAddr,&nBufferSize,NULL,NULL);
		if (hConnectSocket==INVALID_SOCKET) return false;

		//获取连接
		pServerSocketItem=m_pTCPSocketManager->ActiveSocketItem();
		if (pServerSocketItem==NULL) throw TEXT("申请连接对象失败");

		//激活对象
		CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
		pServerSocketItem->Attach(hConnectSocket,SocketAddr.sin_addr.S_un.S_addr);
		CreateIoCompletionPort((HANDLE)hConnectSocket,m_hCompletionPort,(ULONG_PTR)pServerSocketItem,0);
		pServerSocketItem->RecvData();
	}
	catch (...)
	{
		//清理对象
		ASSERT(pServerSocketItem==NULL);
		if (hConnectSocket!=INVALID_SOCKET)	closesocket(hConnectSocket);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CSocketDetectThread::CSocketDetectThread(void)
{
	m_dwTickCount=0;;
	m_pTCPSocketManager=NULL;
}

//析构函数
CSocketDetectThread::~CSocketDetectThread(void)
{
}

//配置函数
bool CSocketDetectThread::InitThread(CTCPSocketEngine * pTCPSocketManager)
{
	//效验参数
	ASSERT(pTCPSocketManager!=NULL);

	//设置变量
	m_dwTickCount=0L;
	m_pTCPSocketManager=pTCPSocketManager;

	return true;
}

//运行函数
bool CSocketDetectThread::RepetitionRun()
{
	//效验参数
	ASSERT(m_pTCPSocketManager!=NULL);

	//设置间隔
	Sleep(500);
	m_dwTickCount += 500L;

	//检测连接
	if (m_dwTickCount>20000L)
	{
		m_dwTickCount=0L;
		m_pTCPSocketManager->DetectSocket();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CTCPSocketEngine::CTCPSocketEngine(void)
{
	m_bService=false;
	m_bNoDelayMod=false;
	m_wListenPort=0;
	m_dwLastDetect=0;
	m_wMaxSocketItem=0;
	m_hCompletionPort=NULL;
	m_hServerSocket=INVALID_SOCKET;

	//初始化 SOCKET
	WSADATA WSAData;
	WSAStartup(WINSOCK_VERSION,&WSAData);
}

//析构函数
CTCPSocketEngine::~CTCPSocketEngine(void)
{
	//停止服务
	EndService();

	//释放存储连接
	for(CServerSocketItemPtrMap::iterator iter = m_StorageSocketItem.begin();
		iter != m_StorageSocketItem.end(); ++iter)
	{
		delete iter->second;
	}

	//清理 SOCKET
	WSACleanup();
}

//设置接口
void CTCPSocketEngine::SetSocketEngineSink(CQueueServiceBase * pQueueService)
{
	//状态判断
	if (m_bService==true) 
	{
		return;
	}

	//设置接口
	m_AttemperEvent.SetQueueService(pQueueService);
}

//设置NODELAY模式
void CTCPSocketEngine::SetNoDelayMod(bool bMode)
{
	m_bNoDelayMod = bMode;
}

//设置端口
bool CTCPSocketEngine::SetServicePort(WORD wListenPort)
{
	//效验状态
	if (m_bService==true) 
	{
		return false;
	}

	//判断参数
	if (wListenPort==0)
	{
		return false;
	}

	//设置变量
	m_wListenPort=wListenPort;

	return true;
}

//设置数目
bool CTCPSocketEngine::SetMaxSocketItem(WORD wMaxSocketItem)
{
	m_wMaxSocketItem=wMaxSocketItem;
	return true;
}

//启动服务
bool CTCPSocketEngine::BeginService()
{
	DWORD i = 0;
	//效验状态
	if (m_bService==true) 
	{
		return true;
	}

	//判断端口
	if (m_wListenPort==0)
	{
		return false;
	}

	//绑定对象
	m_SendQueueService.SetQueueServiceSink(this);

	try
	{
		//获取系统信息
		SYSTEM_INFO SystemInfo;
		GetSystemInfo(&SystemInfo);
		DWORD dwThreadCount=SystemInfo.dwNumberOfProcessors;

		//建立完成端口
		m_hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
		if (m_hCompletionPort==NULL) throw TEXT("网络引擎完成端口创建失败");

		//建立监听SOCKET
		struct sockaddr_in SocketAddr;
		memset(&SocketAddr,0,sizeof(SocketAddr));
		SocketAddr.sin_addr.s_addr=INADDR_ANY;
		SocketAddr.sin_family=AF_INET;
		SocketAddr.sin_port=htons(m_wListenPort);
		m_hServerSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
		if (m_hServerSocket==INVALID_SOCKET) throw TEXT("网络引擎监听 SOCKET 创建失败");
		int iErrorCode=bind(m_hServerSocket,(SOCKADDR*)&SocketAddr,sizeof(SocketAddr));
		if (iErrorCode==SOCKET_ERROR) throw TEXT("网络引擎监听端口被占用，端口绑定失败");
		iErrorCode=listen(m_hServerSocket,SOMAXCONN);
		if (iErrorCode==SOCKET_ERROR) throw TEXT("网络引擎监听端口被占用，端口监听失败");

		//启动发送队列
		bool bSuccess=m_SendQueueService.BeginService();
		if (bSuccess==false) throw TEXT("网络引擎发送队列服务启动失败");

		//建立读写线程
		for (i=0;i<dwThreadCount;i++)
		{
			CServerSocketRSThread * pServerSocketRSThread=new CServerSocketRSThread();
			if (pServerSocketRSThread==NULL) throw TEXT("网络引擎读写线程服务创建失败");
			bSuccess=pServerSocketRSThread->InitThread(m_hCompletionPort);
			if (bSuccess==false) throw TEXT("网络引擎读写线程服务配置失败");
			m_SocketRSThreadArray.push_back(pServerSocketRSThread);
		}

		//建立应答线程
		bSuccess=m_SocketAcceptThread.InitThread(m_hCompletionPort,m_hServerSocket,this);
		if (bSuccess==false) throw TEXT("网络引擎网络监听线程服务配置");

		//运行读写线程
		for (i=0;i<dwThreadCount;i++)
		{
			CServerSocketRSThread * pServerSocketRSThread=m_SocketRSThreadArray[i];
			ASSERT(pServerSocketRSThread!=NULL);
			bSuccess=pServerSocketRSThread->StartThead();
			if (bSuccess==false) throw TEXT("网络引擎读写线程服务启动失败");
		}

		//网络检测线程
		m_SocketDetectThread.InitThread(this);
		bSuccess=m_SocketDetectThread.StartThead();
		if (bSuccess==false) throw TEXT("网络引擎检测线程服务启动失败");

		//运行应答线程
		bSuccess=m_SocketAcceptThread.StartThead();
		if (bSuccess==false) throw TEXT("网络引擎监听线程服务启动失败");

		//设置变量
		m_bService=true;
	}
	catch (std::string &strError)
	{
		CLogEngine::Instance().Log(Level_Exception, strError.c_str());
		return false;
	}

	return true;
}

//停止服务
bool CTCPSocketEngine::EndService()
{
	//设置变量
	m_bService=false;
	m_dwLastDetect=0L;

	//停止检测线程
	m_SocketDetectThread.StopThread();

	//终止应答线程
	if (m_hServerSocket!=INVALID_SOCKET)
	{
		closesocket(m_hServerSocket);
		m_hServerSocket=INVALID_SOCKET;
	}
	m_SocketAcceptThread.StopThread();

	//停止发送队列
	m_SendQueueService.EndService();

	//释放读写线程
	int nCount=m_SocketRSThreadArray.size(),i=0;
	if (m_hCompletionPort!=NULL)
	{
		for (i=0;i<nCount;i++) PostQueuedCompletionStatus(m_hCompletionPort,0,NULL,NULL);
	}
	for (i=0;i<nCount;i++)
	{
		CServerSocketRSThread * pSocketThread=m_SocketRSThreadArray[i];
		ASSERT(pSocketThread!=NULL);
		pSocketThread->StopThread();
		SAFE_DELETE(pSocketThread);
	}
	m_SocketRSThreadArray.clear();

	//关闭完成端口
	if (m_hCompletionPort!=NULL)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort=NULL;
	}

	//关闭连接
	CServerSocketItem *pServerSocketItem=NULL;
	for (CServerSocketItemPtrSet::iterator iter = m_ActiveSocketItem.begin();
		iter != m_ActiveSocketItem.end(); ++iter)
	{
		pServerSocketItem=*iter;
		ASSERT(pServerSocketItem!=NULL);
		pServerSocketItem->CloseSocket(pServerSocketItem->GetRountID());
		pServerSocketItem->ResetSocketData();
	}
	m_FreeSocketItem.insert(m_ActiveSocketItem.begin(), m_ActiveSocketItem.end());
	m_ActiveSocketItem.clear();

	return true;
}

//应答消息
bool CTCPSocketEngine::OnSocketAcceptEvent(CServerSocketItem * pServerSocketItem)
{
	//效验数据
	ASSERT(pServerSocketItem!=NULL);
	if (NULL == pServerSocketItem) return false;

	if(m_bNoDelayMod)
	{
		static const int ndoption = 1;
		setsockopt(pServerSocketItem->GetHandle(), IPPROTO_TCP, TCP_NODELAY, (const char*)&ndoption, sizeof(ndoption));
	}

	//投递消息
	if (m_bService==false) return false;
	m_AttemperEvent.PostSocketAcceptEvent(pServerSocketItem->GetSocketID(),pServerSocketItem->GetClientAddr());

	return true;
}

//网络读取消息
bool CTCPSocketEngine::OnSocketReadEvent(DWORD dwCommand, void * pBuffer, WORD wDataSize, CServerSocketItem * pServerSocketItem)
{
	//效验数据
	ASSERT(pServerSocketItem!=NULL);
	if (NULL == pServerSocketItem) return false;

	//效验状态
	if (m_bService==false) return false;
	m_AttemperEvent.PostSocketReadEvent(pServerSocketItem->GetSocketID(),dwCommand,pBuffer,wDataSize);

	return true;
}

//网络关闭消息
bool CTCPSocketEngine::OnSocketCloseEvent(CServerSocketItem * pServerSocketItem)
{
	//效验参数
	ASSERT(pServerSocketItem!=NULL);
	if (NULL == pServerSocketItem) return false;

	try
	{
		//效验状态
		if (m_bService==false) return false;

		//计算时间
		DWORD dwClientAddr=pServerSocketItem->GetClientAddr();
		DWORD dwConnectTime=pServerSocketItem->GetConnectTime();
		m_AttemperEvent.PostSocketCloseEvent(pServerSocketItem->GetSocketID(),dwClientAddr,dwConnectTime);

		//释放连接
		FreeSocketItem(pServerSocketItem);
	}
	catch (...) {}

	return true;
}

//通知回调函数（发送队列线程调用）
void CTCPSocketEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	switch (wIdentifier)
	{
	case QUEUE_SEND_REQUEST:		//发送请求
		{
			//效验数据
			tagSendDataRequest * pSendDataRequest=(tagSendDataRequest *)pBuffer;
			ASSERT(wDataSize>=(sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuf)));
			ASSERT(wDataSize==(pSendDataRequest->wDataSize+sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuf)));

			//发送数据
			if (pSendDataRequest->wIndex==INDEX_ALL_SOCKET)
			{
				//获取活动项
				CThreadLockHandle ItemLockedHandle(&m_ItemLocked);
				m_TempSocketItem.clear();
				m_TempSocketItem.insert(m_ActiveSocketItem.begin(), m_ActiveSocketItem.end());
				ItemLockedHandle.UnLock();

				//循环发送数据
				CServerSocketItem * pServerSocketItem=NULL;
				for (CServerSocketItemPtrSet::iterator iter = m_TempSocketItem.begin();
					iter != m_TempSocketItem.end(); ++iter)
				{
					pServerSocketItem=*iter;
					ASSERT(pServerSocketItem!=NULL);
					CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
					pServerSocketItem->SendData(pSendDataRequest->dwCommand,pSendDataRequest->cbSendBuf,pSendDataRequest->wDataSize,pServerSocketItem->GetRountID());
				}
			}
			else
			{
				//单项发送
				CServerSocketItem * pServerSocketItem=EnumSocketItem(pSendDataRequest->wIndex);
				CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
				pServerSocketItem->SendData(pSendDataRequest->dwCommand,pSendDataRequest->cbSendBuf,pSendDataRequest->wDataSize,pServerSocketItem->GetRountID());
			}

			break;
		}
	case QUEUE_SAFE_CLOSE:		//安全关闭
		{
			//效验数据
			ASSERT(wDataSize==sizeof(tagSafeCloseSocket));
			tagSafeCloseSocket * pSafeCloseSocket=(tagSafeCloseSocket *)pBuffer;

			//安全关闭
			CServerSocketItem * pServerSocketItem=EnumSocketItem(pSafeCloseSocket->wIndex);
			CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
			pServerSocketItem->ShutDownSocket(pSafeCloseSocket->wRountID);

			break;
		}
	case QUEUE_DETECT_SOCKET:	//检测连接
		{
			//获取活动项
			CThreadLockHandle ItemLockedHandle(&m_ItemLocked);
			m_TempSocketItem.clear();
			m_TempSocketItem.insert(m_ActiveSocketItem.begin(), m_ActiveSocketItem.end());
			ItemLockedHandle.UnLock();

			//构造数据
			CMD_KN_DetectSocket DetectSocket;
			ZeroMemory(&DetectSocket,sizeof(DetectSocket));

			//变量定义
			WORD wRountID=0;
			DWORD dwNowTickCount=GetTickCount();
			DWORD dwBreakTickCount=__max(dwNowTickCount-m_dwLastDetect,TIME_BREAK_READY);

			//设置变量
			m_dwLastDetect=GetTickCount();

			//检测连接
			CServerSocketItem * pServerSocketItem=NULL;
			for (CServerSocketItemPtrSet::iterator iter = m_TempSocketItem.begin();
				iter != m_TempSocketItem.end(); ++iter)
			{
				//变量定义
				pServerSocketItem=*iter;
				DWORD dwRecvTickCount=pServerSocketItem->GetRecvTickCount();
				CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());

				//间隔检查
				if (dwRecvTickCount>=dwNowTickCount) continue;

				//检测连接
				if (pServerSocketItem->IsReadySend()==true)
				{
					if ((dwNowTickCount-dwRecvTickCount)>dwBreakTickCount)
					{
						pServerSocketItem->CloseSocket(pServerSocketItem->GetRountID());
						continue;
					}
				}
				else
				{
					if ((dwNowTickCount-dwRecvTickCount)>TIME_BREAK_CONNECT)
					{
						pServerSocketItem->CloseSocket(pServerSocketItem->GetRountID());
						continue;
					}
				}

				//发送数据
				if (pServerSocketItem->IsReadySend()==true)
				{
					wRountID=pServerSocketItem->GetRountID();
					DetectSocket.dwSendTickCount=GetTickCount();
					pServerSocketItem->SendData(CMD_DETECT_SOCKET, &DetectSocket,sizeof(DetectSocket),wRountID);
				}
			}

			break;
		}
	default: { ASSERT(FALSE); }
	}

	return;
}

//获取空闲对象
CServerSocketItem * CTCPSocketEngine::ActiveSocketItem()
{
	CThreadLockHandle ItemLockedHandle(&m_ItemLocked,true);

	//获取空闲对象
	CServerSocketItem * pServerSocketItem=NULL;
	if (m_FreeSocketItem.size()>0)
	{
		pServerSocketItem=*m_FreeSocketItem.begin();
		ASSERT(pServerSocketItem!=NULL);
		m_FreeSocketItem.erase(m_FreeSocketItem.begin());
		m_ActiveSocketItem.insert(pServerSocketItem);
	}

	//创建新对象
	if (pServerSocketItem==NULL)
	{
		WORD wStorageCount=(WORD)m_StorageSocketItem.size();
		if (wStorageCount<m_wMaxSocketItem)
		{
			try
			{
				pServerSocketItem=new CServerSocketItem(wStorageCount,static_cast<CServerSocketItemSink*>(this));
				if (pServerSocketItem==NULL) return NULL;
				m_StorageSocketItem.insert(CServerSocketItemPtrMap::_Val_type(wStorageCount, pServerSocketItem));
				m_ActiveSocketItem.insert(pServerSocketItem);
			}
			catch (...) { return NULL; }
		}
	}

	return pServerSocketItem;
}

//获取连接对象
CServerSocketItem * CTCPSocketEngine::EnumSocketItem(WORD wIndex)
{
	CThreadLockHandle ItemLockedHandle(&m_ItemLocked,true);
	CServerSocketItemPtrMap::iterator iter = m_StorageSocketItem.find(wIndex);
	if (iter != m_StorageSocketItem.end())
	{
		CServerSocketItem * pServerSocketItem=iter->second;
		ASSERT(pServerSocketItem!=NULL);
		return pServerSocketItem;
	}
	return NULL;
}

//释放连接对象
bool CTCPSocketEngine::FreeSocketItem(CServerSocketItem * pServerSocketItem)
{
	//效验参数
	ASSERT(pServerSocketItem!=NULL);

	//释放对象
	CThreadLockHandle ItemLockedHandle(&m_ItemLocked,true);

	CServerSocketItemPtrSet::iterator iter = m_ActiveSocketItem.find(pServerSocketItem);
	if(iter != m_ActiveSocketItem.end())
	{
		m_FreeSocketItem.insert(pServerSocketItem);
		m_ActiveSocketItem.erase(iter);
		return true;
	}

	//释放失败
	ASSERT(FALSE);
	return false;
}

//检测连接
bool CTCPSocketEngine::DetectSocket()
{
	return m_SendQueueService.AddToQueue(QUEUE_DETECT_SOCKET,NULL,0);
}

//发送函数
bool CTCPSocketEngine::SendData(DWORD dwSocketID, DWORD dwCommand, void * pData, WORD wDataSize)
{
	//效益状态
	ASSERT(m_bService==true);
	if (m_bService==false) return false;

	//效益数据
	ASSERT((wDataSize+sizeof(CMD_Head))<=SOCKET_PACKET);
	if ((wDataSize+sizeof(CMD_Head))>SOCKET_PACKET) return false;

	//构造数据
	tagSendDataRequest SendRequest;
	SendRequest.dwCommand=dwCommand;
	SendRequest.wIndex=GetIndexFromeSocketID(dwSocketID);
	SendRequest.wRountID=GetRoundIDFromSocketID(dwSocketID);
	SendRequest.wDataSize=wDataSize;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(SendRequest.cbSendBuf,pData,wDataSize);
	}

	//加入发送队列
	WORD wSendSize=sizeof(SendRequest)-sizeof(SendRequest.cbSendBuf)+wDataSize;
	return m_SendQueueService.AddToQueue(QUEUE_SEND_REQUEST,&SendRequest,wSendSize);
}

//批量发送
bool CTCPSocketEngine::SendDataBatch(DWORD dwCommand, void * pData, WORD wDataSize)
{
	//效益状态
	if (m_bService==false) return false;

	//效益数据
	ASSERT((wDataSize+sizeof(CMD_Head))<=SOCKET_PACKET);
	if ((wDataSize+sizeof(CMD_Head))>SOCKET_PACKET) return false;

	//构造数据
	tagSendDataRequest SendRequest;
	SendRequest.dwCommand=dwCommand;
	SendRequest.wIndex=INDEX_ALL_SOCKET;
	SendRequest.wRountID=0;
	SendRequest.wDataSize=wDataSize;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(SendRequest.cbSendBuf,pData,wDataSize);
	}

	//加入发送队列
	WORD wSendSize=sizeof(SendRequest)-sizeof(SendRequest.cbSendBuf)+wDataSize;
	return m_SendQueueService.AddToQueue(QUEUE_SEND_REQUEST,&SendRequest,wSendSize);
}

//关闭连接
bool CTCPSocketEngine::CloseSocket(DWORD dwSocketID)
{
	CServerSocketItem * pServerSocketItem=EnumSocketItem(GetIndexFromeSocketID(dwSocketID));
	if (pServerSocketItem==NULL) return false;
	CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
	return pServerSocketItem->CloseSocket(GetRoundIDFromSocketID(dwSocketID));
}

//设置关闭
bool CTCPSocketEngine::ShutDownSocket(DWORD dwSocketID)
{
	tagSafeCloseSocket SafeCloseSocket;
	SafeCloseSocket.wIndex=GetIndexFromeSocketID(dwSocketID);
	SafeCloseSocket.wRountID=GetRoundIDFromSocketID(dwSocketID);
	return m_SendQueueService.AddToQueue(QUEUE_SAFE_CLOSE,&SafeCloseSocket,sizeof(SafeCloseSocket));
}

//////////////////////////////////////////////////////////////////////////