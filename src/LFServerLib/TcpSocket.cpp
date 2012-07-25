#include "TcpSocket.h"
#include "FormatString.h"
#include "BigNumber.h"

//////////////////////////////////////////////////////////////////////////

//构造函数
CSocketThread::CSocketThread()
{
	m_pTcpSocket = NULL;
}

//析构函数
CSocketThread::~CSocketThread()
{
}

//配置函数
bool CSocketThread::InitThread(CTcpSocket * pTcpSocket)
{
	if(pTcpSocket==NULL) return false;

	m_pTcpSocket = pTcpSocket;

	return true;
}

//运行函数
bool CSocketThread::RepetitionRun()
{
	ASSERT(m_pTcpSocket!=NULL);

	WaitForSingleObject(m_pTcpSocket->m_hSocketEvent, INFINITE);

	WSANETWORKEVENTS tempEvent; 
	WSAEnumNetworkEvents(m_pTcpSocket->m_hSocket, m_pTcpSocket->m_hSocketEvent, &tempEvent);

	if(tempEvent.lNetworkEvents & FD_CONNECT)
	{
		m_pTcpSocket->OnSocketNotifyConnect(tempEvent.iErrorCode[FD_CONNECT_BIT]);
	}
	else if(tempEvent.lNetworkEvents & FD_READ)
	{
		m_pTcpSocket->OnSocketNotifyRead(tempEvent.iErrorCode[FD_READ_BIT]);
	}
	else if(tempEvent.lNetworkEvents & FD_CLOSE)
	{
		m_pTcpSocket->OnSocketNotifyClose(tempEvent.iErrorCode[FD_CLOSE_BIT]);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

CTcpSocket::CTcpSocket()
{
	m_wRecvSize=0;
	m_dwSendTickCount=0;
	m_dwRecvTickCount=0;
	m_dwSendPacketCount=0;
	m_dwRecvPacketCount=0;
	m_bCloseByServer=false;
	m_hSocket=INVALID_SOCKET;
	m_hSocketEvent = NULL;
	m_pTcpSocketSink=NULL;
	m_SocketState=SocketState_NoConnect;

	BigNumber bn;
	bn.SetHexStr(COMMAND_AUTHCRYPT_KEY);
	m_AuthClientCrypt.Init(&bn);

	//初始化 SOCKET
	WSADATA WSAData;
	WSAStartup(WINSOCK_VERSION,&WSAData);

	m_hSocketEvent = WSACreateEvent();
}

CTcpSocket::~CTcpSocket()
{
	CloseSocket(false);
	SetEvent(m_hSocketEvent);

	if(m_SocketThread.IsRuning())
	{
		m_SocketThread.StopThread();
	}
	if(m_hSocketEvent!=NULL)
	{
		WSACloseEvent(m_hSocketEvent);
		m_hSocketEvent = NULL;
	}

	//清理 SOCKET
	WSACleanup();
}

//设置接口
void CTcpSocket::SetSocketSink(CTcpSocketSink *pTcpSocketSink)
{
	ASSERT(pTcpSocketSink!=NULL);
	m_pTcpSocketSink = pTcpSocketSink;
}

//设置NODELAY模式
bool CTcpSocket::SetNoDelayMod(bool bMode)
{
	int ndoption = 0;
	if(bMode) ndoption = 1;
	int ret = setsockopt(m_hSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&ndoption, sizeof(ndoption));
	if(ret == -1)
	{
		return false;
	}
	return true;
}

//连接服务器
bool CTcpSocket::Connect(DWORD dwServerIP, WORD wPort)
{
	//效验参数
	ASSERT(m_hSocket==INVALID_SOCKET);
	ASSERT(m_SocketState==SocketState_NoConnect);

	//效验状态
	if (m_hSocket!=INVALID_SOCKET) throw TEXT("连接 SOCKET 句柄已经存在");
	if (m_SocketState!=SocketState_NoConnect) throw TEXT("连接状态不是等待连接状态");
	if (dwServerIP==INADDR_NONE) throw TEXT("目标服务器地址格式不正确，请检查后再次尝试！");

	//设置参数
	m_wRecvSize=0;
	m_dwSendTickCount=GetTickCount()/1000L;
	m_dwRecvTickCount=GetTickCount()/1000L;

	try
	{
		//建立 SOCKET
		m_hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if (m_hSocket==INVALID_SOCKET) throw TEXT("SOCKET 创建失败");

		//填写服务器地址
		SOCKADDR_IN SocketAddr;
		memset(&SocketAddr,0,sizeof(SocketAddr));
		SocketAddr.sin_family=AF_INET;
		SocketAddr.sin_port=htons(wPort);
		SocketAddr.sin_addr.S_un.S_addr=dwServerIP;

		//连接服务器
		int iErrorCode=0;
		WSASetLastError(0);

		//创建SOCKET事件
		iErrorCode = WSAEventSelect(m_hSocket, m_hSocketEvent, FD_CONNECT|FD_READ|FD_CLOSE);
		if (iErrorCode==SOCKET_ERROR) throw TEXT("绑定网络事件错误");

		//连接服务器
		iErrorCode=connect(m_hSocket,(SOCKADDR *)&SocketAddr,sizeof(SocketAddr));
		if (iErrorCode==SOCKET_ERROR)
		{
			iErrorCode=WSAGetLastError();
			if (iErrorCode!=WSAEWOULDBLOCK)
			{
				std::wstring strError = std::move(wstrFormat(L"连接发生错误，错误代码 [ %d ]", iErrorCode));
				throw strError;
			}
		}

		//设置变量
		m_SocketState=SocketState_Connecting;

		//开启网络消息线程
		m_SocketThread.InitThread(this);
		m_SocketThread.StartThead();

		return true;
	}
	catch(std::string &strError)
	{
		CloseSocket(false);
		throw strError;
	}
	catch(...)
	{
		CloseSocket(false);
		throw TEXT("连接产生未知异常错误");
	}
	return false;
}

//连接服务器
bool CTcpSocket::Connect(const std::wstring& strServerIP, WORD wPort)
{
	//效验数据
	ASSERT(wPort!=0);
	ASSERT(strServerIP.empty()==false);
	if (wPort==0) return false;
	if (strServerIP.empty()) return false;
	return Connect(TranslateAddr(strServerIP),wPort);
}

//发送函数
bool CTcpSocket::SendData(DWORD dwCommand, void * pData, WORD wDataSize)
{
	//效验状态
	if (m_hSocket==INVALID_SOCKET) return false;
	if (m_SocketState!=SocketState_Connected) return false;

	//效验大小
	ASSERT(wDataSize<=SOCKET_PACKET);
	if (wDataSize>SOCKET_PACKET) return false;

	//构造数据
	BYTE cbDataBuffer[SOCKET_BUFFER];
	CMD_Head * pHead=(CMD_Head *)cbDataBuffer;
	pHead->dwCommand=dwCommand;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(pHead+1,pData,wDataSize);
	}

	//加密数据
	WORD wSendSize=EncryptBuffer(cbDataBuffer,sizeof(CMD_Head)+wDataSize,sizeof(cbDataBuffer));

	//发送数据
	return SendBuffer(cbDataBuffer,wSendSize);
}

//关闭连接
bool CTcpSocket::CloseSocket(bool bNotify)
{
	//关闭连接
	bool bClose=(m_hSocket!=INVALID_SOCKET);
	m_SocketState=SocketState_NoConnect;
	if (m_hSocket!=INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket=INVALID_SOCKET;
		m_SocketState=SocketState_NoConnect;
	}
	if ((bNotify==true)&&(bClose==true)&&(m_pTcpSocketSink!=NULL))
	{
		ASSERT(m_pTcpSocketSink!=NULL);
		try { m_pTcpSocketSink->OnSocketClose(this,m_bCloseByServer); }
		catch (...) {}
	}

	//恢复数据
	m_wRecvSize=0;
	m_dwSendTickCount=0;
	m_dwRecvTickCount=0;
	m_dwSendPacketCount=0;
	m_dwRecvPacketCount=0;
	m_bCloseByServer=false;

	return true;
}

//发送数据
bool CTcpSocket::SendBuffer(void * pBuffer, WORD wSendSize)
{
	//效验参数
	ASSERT(wSendSize!=0);
	ASSERT(pBuffer!=NULL);

	//发送数据
	WORD wSended=0;
	while (wSended<wSendSize)
	{
		int iErrorCode=send(m_hSocket,(char *)pBuffer+wSended,wSendSize-wSended,0);
		if (iErrorCode==SOCKET_ERROR)
		{
			int n = WSAGetLastError();
			if (WSAGetLastError()==WSAEWOULDBLOCK)
			{
				m_dwSendTickCount=GetTickCount()/1000L;
				return true;
			}
			return false;
		}
		wSended+=iErrorCode;
	}
	m_dwSendTickCount=GetTickCount()/1000L;

	return true;
}

//解释服务器地址
DWORD CTcpSocket::TranslateAddr(const std::wstring& strServerAddr)
{
	std::string strAddr = UnicodeToMB(strServerAddr);
	DWORD dwServerIP=inet_addr(strAddr.c_str());
	if (dwServerIP==INADDR_NONE)
	{
		LPHOSTENT lpHost=gethostbyname(strAddr.c_str());
		if (lpHost==NULL) return INADDR_NONE;
		dwServerIP=((LPIN_ADDR)lpHost->h_addr)->s_addr;
	}
	return dwServerIP;
}

//解释错误
std::wstring CTcpSocket::GetConnectError(int iErrorCode,std::wstring& strBuffer)
{
	//解释错误
	switch (iErrorCode)
	{
	case 0:					//没有错误
		{
			strBuffer = L"操作执行成功！";
			break;
		}
	case WSAEADDRNOTAVAIL:	//地址格式错误
		{
			strBuffer = L"目标服务器地址格式不正确，请检查后再次尝试！";
			break;
		}
	case WSAECONNREFUSED:	//服务器没有启动
		{
			strBuffer = L"目标服务器繁忙或者没有启动！";
			break;
		}
	case WSAETIMEDOUT:		//连接超时
		{
			strBuffer = L"连接超时，可能是目标服务器不存在或者服务器地址格式不正确！";
			break;
		}
	case WSAEHOSTUNREACH:
		{
			strBuffer = L"网络连接失败，请检查是否已经成功拨号和连接 Internet ！";
			break;
		}
	default:				//默认错误
		{
			strBuffer = std::move(wstrFormat(L"连接错误号：%ld，详细错误信息请参考操作帮助手册！",iErrorCode));
			break;
		}
	}
	return strBuffer;
}

//加密数据
WORD CTcpSocket::EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize)
{
	//效验参数
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(wBufferSize>=(wDataSize+2*sizeof(DWORD)));
	ASSERT(wDataSize<=(sizeof(CMD_Head)+SOCKET_PACKET));

	//效验码
	BYTE cbCheckCode=0;
	for (WORD i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
	}

	//填写信息头
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	pHead->CmdInfo.cbCheckCode=~cbCheckCode+1;
	pHead->CmdInfo.wDataSize=wDataSize;
	pHead->CmdInfo.cbMessageVer=SOCKET_VER;

	//加密数据-只加密Command
	m_AuthClientCrypt.EncryptSend(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//设置变量
	m_dwSendPacketCount++;

	return wDataSize;
}

//解密数据
WORD CTcpSocket::CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize)
{
	//效验参数
	ASSERT(m_dwSendPacketCount>0);
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(((CMD_Head *)pcbDataBuffer)->CmdInfo.wDataSize==wDataSize);

	//解密数据
	m_AuthClientCrypt.DecryptRecv(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//效验码
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	BYTE cbCheckCode=pHead->CmdInfo.cbCheckCode;
	for (int i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
	}
	if (cbCheckCode!=0) throw TEXT("数据包效验码错误");

	return wDataSize;
}

//网络连接
LRESULT CTcpSocket::OnSocketNotifyConnect(int iErrorCode)
{
	//判断状态
	if (iErrorCode==0) m_SocketState=SocketState_Connected;
	else CloseSocket(false);

	//发送通知
	std::wstring strErrorDesc;
	GetConnectError(iErrorCode,strErrorDesc);
	m_pTcpSocketSink->OnSocketConnect(iErrorCode,strErrorDesc,this);

	return 1;
}

//网络读取
LRESULT CTcpSocket::OnSocketNotifyRead(int iErrorCode)
{
	try
	{
		//读取数据
		int iRetCode=recv(m_hSocket,(char *)m_cbRecvBuf+m_wRecvSize,sizeof(m_cbRecvBuf)-m_wRecvSize,0);
		if (iRetCode==SOCKET_ERROR) throw TEXT("网络连接关闭，读取数据失败");
		ASSERT(m_dwSendPacketCount>0);
		m_wRecvSize+=iRetCode;
		m_dwRecvTickCount=GetTickCount()/1000L;

		//变量定义
		WORD wPacketSize=0;
		BYTE cbDataBuffer[SOCKET_PACKET+sizeof(CMD_Head)];
		CMD_Head * pHead=(CMD_Head *)m_cbRecvBuf;

		while (m_wRecvSize>=sizeof(CMD_Head))
		{
			//效验参数
			wPacketSize=pHead->CmdInfo.wDataSize;
			ASSERT(pHead->CmdInfo.cbMessageVer==SOCKET_VER);
			ASSERT(wPacketSize<=(SOCKET_PACKET+sizeof(CMD_Head)));
			if (pHead->CmdInfo.cbMessageVer!=SOCKET_VER) throw TEXT("数据包版本错误");
			if (wPacketSize>(SOCKET_PACKET+sizeof(CMD_Head))) throw TEXT("数据包太大");
			if (m_wRecvSize<wPacketSize) return 1;

			//拷贝数据
			m_dwRecvPacketCount++;
			CopyMemory(cbDataBuffer,m_cbRecvBuf,wPacketSize);
			m_wRecvSize-=wPacketSize;
			MoveMemory(m_cbRecvBuf,m_cbRecvBuf+wPacketSize,m_wRecvSize);

			//解密数据
			WORD wRealySize=CrevasseBuffer(cbDataBuffer,wPacketSize);
			ASSERT(wRealySize>=sizeof(CMD_Head));

			//解释数据
			WORD wDataSize=wRealySize-sizeof(CMD_Head);
			void * pDataBuffer=cbDataBuffer+sizeof(CMD_Head);
			DWORD dwCommand=((CMD_Head *)cbDataBuffer)->dwCommand;

			//网络检测
			if (dwCommand==CMD_DETECT_SOCKET)
			{
						//发送数据
				SendData(CMD_DETECT_SOCKET,pDataBuffer,wDataSize);
				continue;
			}

			//处理数据
			bool bSuccess=m_pTcpSocketSink->OnSocketRead(dwCommand,pDataBuffer,wDataSize,this);
			if (bSuccess==false) throw TEXT("网络数据包处理失败");
		};
	}
	catch (...) 
	{ 
		CloseSocket(true); 
	}

	return 1;
}

//网络关闭
LRESULT CTcpSocket::OnSocketNotifyClose(int iErrorCode)
{
	m_bCloseByServer=true;
	CloseSocket(true);
	return 1;
}
//////////////////////////////////////////////////////////////////////////