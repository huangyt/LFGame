#include "SocketEngine.h"
#include "LogEngine.h"
#include "BigNumber.h"

//////////////////////////////////////////////////////////////////////////
//�궨��
#define INDEX_ALL_SOCKET			0xFFFF								//��������

#define TIME_BREAK_READY			9000L								//�ж�ʱ��
#define TIME_BREAK_CONNECT			4000L								//�ж�ʱ��
#define TIME_DETECT_SOCKET			20000L								//���ʱ��

//////////////////////////////////////////////////////////////////////////

//��������
#define QUEUE_SEND_REQUEST			1									//���ͱ�ʶ
#define QUEUE_SAFE_CLOSE			2									//��ȫ�ر�
#define QUEUE_DETECT_SOCKET			3									//�������

//��������
#define GetIndexFromeSocketID(a)	LOWORD(a)
#define GetRoundIDFromSocketID(a)	HIWORD(a)

//��������ṹ
struct tagSendDataRequest
{
	DWORD							dwCommand;							//����
	WORD							wIndex;								//��������
	WORD							wRountID;							//ѭ������
	WORD							wDataSize;							//���ݴ�С
	BYTE							cbSendBuf[SOCKET_BUFFER];			//���ͻ���
};

//��ȫ�ر�
struct tagSafeCloseSocket
{
	WORD							wIndex;								//��������
	WORD							wRountID;							//ѭ������
};

//////////////////////////////////////////////////////////////////////////

//���캯��
COverLapped::COverLapped(enOperationType OperationType) : m_OperationType(OperationType)
{
	memset(&m_WSABuffer,0,sizeof(m_WSABuffer));
	memset(&m_OverLapped,0,sizeof(m_OverLapped));
}

//��������
COverLapped::~COverLapped()
{
}

//////////////////////////////////////////////////////////////////////////

//���캯��
COverLappedSend::COverLappedSend() : COverLapped(OperationType_Send)
{
	m_WSABuffer.len=0;
	m_WSABuffer.buf=(char *)m_cbBuffer;
}

//��������
COverLappedSend::~COverLappedSend()
{
}

//////////////////////////////////////////////////////////////////////////
//���캯��
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

//��������
CServerSocketItem::~CServerSocketItem(void)
{
	//ɾ�������ص� IO
	for(COverLappedSendSet::iterator iter = m_OverLappedSendFree.begin();
		iter != m_OverLappedSendFree.end(); ++iter)
	{
		delete *iter;
	}

	//ɾ����ص� IO
	for(COverLappedSendSet::iterator iter = m_OverLappedSendActive.begin();
		iter != m_OverLappedSendActive.end(); ++iter)
	{
		delete *iter;
	}
}

//��ȡ���ͽṹ
COverLappedSend * CServerSocketItem::GetSendOverLapped()
{
	//Ѱ�ҿ��нṹ
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

	//�½����ͽṹ
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

//�󶨶���
DWORD CServerSocketItem::Attach(SOCKET hSocket, DWORD dwClientAddr)
{
	//Ч������
	ASSERT(dwClientAddr!=0);
	ASSERT(m_bRecvIng==false);
	ASSERT(IsValidSocket()==false);
	ASSERT(hSocket!=INVALID_SOCKET);

	//���ñ���
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

	//����֪ͨ
	m_pServerSocketItemSink->OnSocketAcceptEvent(this);

	return GetSocketID();
}

//���ͺ���
bool CServerSocketItem::SendData(DWORD dwCommand, void * pData, WORD wDataSize, WORD wRountID)
{
	//Ч�����
	ASSERT(wDataSize<=SOCKET_BUFFER);

	//Ч��״̬
	if (m_bCloseIng==true) return false;
	if (m_wRountID!=wRountID) return false;
	if (m_dwRecvPacketCount==0) return false;
	if (IsValidSocket()==false) return false;
	if (wDataSize>SOCKET_BUFFER) return false;

	//Ѱ�ҷ��ͽṹ
	COverLappedSend * pOverLappedSend=GetSendOverLapped();
	ASSERT(pOverLappedSend!=NULL);
	if (pOverLappedSend==NULL) return false;

	//��������
	CMD_Head * pHead=(CMD_Head *)pOverLappedSend->m_cbBuffer;
	pHead->dwCommand=dwCommand;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		memcpy(pHead+1,pData,wDataSize);
	}
	WORD wSendSize=EncryptBuffer(pOverLappedSend->m_cbBuffer,sizeof(CMD_Head)+wDataSize,sizeof(pOverLappedSend->m_cbBuffer));
	pOverLappedSend->m_WSABuffer.len=wSendSize;

	//��������
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

//Ͷ�ݽ���
bool CServerSocketItem::RecvData()
{
	//Ч�����
	ASSERT(m_bRecvIng==false);
	ASSERT(m_hSocket!=INVALID_SOCKET);

	//�жϹر�
	if (m_bCloseIng==true)
	{
		if (m_OverLappedSendActive.size()==0) CloseSocket(m_wRountID);
		return false;
	}

	//��������
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

//�ر�����
bool CServerSocketItem::CloseSocket(WORD wRountID)
{
	//״̬�ж�
	if (m_wRountID!=wRountID) return false;

	//�ر�����
	if (m_hSocket!=INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket=INVALID_SOCKET;
	}

	//�жϹر�
	if ((m_bRecvIng==false)&&(m_OverLappedSendActive.size()==0)) OnCloseCompleted();

	return true;
}

//���ùر�
bool CServerSocketItem::ShutDownSocket(WORD wRountID)
{
	//״̬�ж�
	if (m_wRountID!=wRountID) return false;
	if (m_hSocket==INVALID_SOCKET) return false;

	//���ñ���
	if (m_bCloseIng==false)
	{
		m_bCloseIng=true;
		if (m_OverLappedSendActive.size()==0) CloseSocket(wRountID);
	}

	return true;
}

//���ñ���
void CServerSocketItem::ResetSocketData()
{
	//Ч��״̬
	ASSERT(m_hSocket==INVALID_SOCKET);

	//��������
	m_wRountID++;
	m_wRecvSize=0;
	m_dwClientAddr=0;
	m_dwConnectTime=0;
	m_dwSendTickCount=0;
	m_dwRecvTickCount=0;
	m_dwSendPacketCount=0;
	m_dwRecvPacketCount=0;

	//״̬����
	m_bNotify=true;
	m_bRecvIng=false;
	m_bCloseIng=false;
	m_OverLappedSendFree.insert(m_OverLappedSendActive.begin(), m_OverLappedSendActive.end());
	m_OverLappedSendActive.clear();

	return;
}

//������ɺ���
bool CServerSocketItem::OnSendCompleted(COverLappedSend * pOverLappedSend, DWORD dwThancferred)
{
	//Ч�����
	ASSERT(pOverLappedSend!=NULL);
	ASSERT(m_OverLappedSendActive.size()>0);
	ASSERT(pOverLappedSend==*m_OverLappedSendActive.begin());

	//�ͷŷ��ͽṹ
	m_OverLappedSendActive.erase(m_OverLappedSendActive.begin());
	m_OverLappedSendFree.insert(pOverLappedSend);

	//���ñ���
	if (dwThancferred!=0) m_dwSendTickCount=GetTickCount();

	//�жϹر�
	if (m_hSocket==INVALID_SOCKET)
	{
		m_OverLappedSendFree.insert(m_OverLappedSendActive.begin(), m_OverLappedSendActive.end());
		m_OverLappedSendActive.clear();
		CloseSocket(m_wRountID);
		return true;
	}

	//������������
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

	//�жϹر�
	if (m_bCloseIng==true) 
		CloseSocket(m_wRountID);

	return true;
}

//������ɺ���
bool CServerSocketItem::OnRecvCompleted(COverLappedRecv * pOverLappedRecv, DWORD dwThancferred)
{
	//Ч������
	ASSERT(m_bRecvIng==true);

	//���ñ���
	m_bRecvIng=false;
	m_dwRecvTickCount=GetTickCount();

	//�жϹر�
	if (m_hSocket==INVALID_SOCKET)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	//��������
	int iRetCode=recv(m_hSocket,(char *)m_cbRecvBuf+m_wRecvSize,sizeof(m_cbRecvBuf)-m_wRecvSize,0);
	if (iRetCode<=0)
	{
		CloseSocket(m_wRountID);
		return true;
	}

	//�������
	m_wRecvSize+=iRetCode;
	BYTE cbBuffer[SOCKET_BUFFER];
	CMD_Head * pHead=(CMD_Head *)m_cbRecvBuf;

	//��������
	try
	{
		while (m_wRecvSize>=sizeof(CMD_Head))
		{
			//Ч������
			WORD wPacketSize=pHead->CmdInfo.wDataSize;
			if (wPacketSize>SOCKET_BUFFER) throw TEXT("���ݰ�����");
			if (wPacketSize<sizeof(CMD_Head)) throw TEXT("���ݰ��Ƿ�");
			if (pHead->CmdInfo.cbMessageVer!=SOCKET_VER) throw TEXT("���ݰ��汾����");
			if (m_wRecvSize<wPacketSize) break;

			//��ȡ����
			CopyMemory(cbBuffer,m_cbRecvBuf,wPacketSize);
			WORD wRealySize=CrevasseBuffer(cbBuffer,wPacketSize);
			ASSERT(wRealySize>=sizeof(CMD_Head));
			m_dwRecvPacketCount++;

			//��������
			WORD wDataSize=wRealySize-sizeof(CMD_Head);
			void * pDataBuffer=cbBuffer+sizeof(CMD_Head);
			DWORD dwCommand=((CMD_Head *)cbBuffer)->dwCommand;

			//������
			if (dwCommand!=CMD_DETECT_SOCKET)
			{
				//��Ϣ����
				m_pServerSocketItemSink->OnSocketReadEvent(dwCommand,pDataBuffer,wDataSize,this);			
			}

			//ɾ����������
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

//�ر����֪ͨ
bool CServerSocketItem::OnCloseCompleted()
{
	//Ч��״̬
	ASSERT(m_hSocket==INVALID_SOCKET);
	ASSERT(m_OverLappedSendActive.size()==0);

	//�ر��¼�
	ASSERT(m_bNotify==false);
	if (m_bNotify==false)
	{
		m_bNotify=true;
		m_pServerSocketItemSink->OnSocketCloseEvent(this);
	}

	//״̬����
	ResetSocketData();

	return true;
}

//��������
WORD CServerSocketItem::EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize)
{
	WORD i = 0;
	//Ч�����
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(wDataSize<=(sizeof(CMD_Head)+SOCKET_PACKET));
	ASSERT(wBufferSize>=(wDataSize+2*sizeof(DWORD)));

	//Ч�������ֽ�ӳ��
	BYTE cbCheckCode=0;
	for (i=sizeof(CMD_Info);i<wDataSize;i++) 
	{
		cbCheckCode+=pcbDataBuffer[i];
	}

	//��д��Ϣͷ
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	pHead->CmdInfo.cbCheckCode=~cbCheckCode+1;
	pHead->CmdInfo.wDataSize=wDataSize;
	pHead->CmdInfo.cbMessageVer=SOCKET_VER;

	//��������-ֻ����Command
	m_AuthServerCrypt.EncryptSend(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//���ñ���
	m_dwSendPacketCount++;

	return wDataSize;
}

//��������
WORD CServerSocketItem::CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize)
{
	//Ч�����
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(((CMD_Head *)pcbDataBuffer)->CmdInfo.wDataSize==wDataSize);

	//��������
	m_AuthServerCrypt.DecryptRecv(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//Ч����
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	BYTE cbCheckCode=pHead->CmdInfo.cbCheckCode;;
	for (int i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
	}
	if (cbCheckCode!=0) throw TEXT("���ݰ�Ч�������");

	return wDataSize;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CServerSocketRSThread::CServerSocketRSThread(void)
{
	m_hCompletionPort=NULL;
}

//��������
CServerSocketRSThread::~CServerSocketRSThread(void)
{
}

//���ú���
bool CServerSocketRSThread::InitThread(HANDLE hCompletionPort)
{
	ASSERT(hCompletionPort!=NULL);
	m_hCompletionPort=hCompletionPort;
	return true;
}

//���к���
bool CServerSocketRSThread::RepetitionRun()
{
	//Ч�����
	ASSERT(m_hCompletionPort!=NULL);

	//��������
	DWORD dwThancferred=0;					
	OVERLAPPED * pOverLapped=NULL;
	COverLapped * pSocketLapped=NULL;
	CServerSocketItem * pServerSocketItem=NULL;

	//�ȴ���ɶ˿�
	BOOL bSuccess=GetQueuedCompletionStatus(m_hCompletionPort,&dwThancferred,(PULONG_PTR)&pServerSocketItem,&pOverLapped,INFINITE);
	if ((bSuccess==FALSE)&&(GetLastError()!=ERROR_NETNAME_DELETED)) return false;
	if ((pServerSocketItem==NULL)&&(pOverLapped==NULL)) return false;

	//�������
	ASSERT(pOverLapped!=NULL);
	ASSERT(pServerSocketItem!=NULL);
	pSocketLapped=CONTAINING_RECORD(pOverLapped,COverLapped,m_OverLapped);
	switch (pSocketLapped->GetOperationType())
	{
	case OperationType_Recv:	//SOCKET ���ݶ�ȡ
		{
			COverLappedRecv * pOverLappedRecv=(COverLappedRecv *)pSocketLapped;
			CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
			pServerSocketItem->OnRecvCompleted(pOverLappedRecv,dwThancferred);
			break;
		}
	case OperationType_Send:	//SOCKET ���ݷ���
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

//���캯��
CSocketAcceptThread::CSocketAcceptThread(void)
{
	m_hCompletionPort=NULL;
	m_pTCPSocketManager=NULL;
	m_hListenSocket=INVALID_SOCKET;
}

//��������
CSocketAcceptThread::~CSocketAcceptThread(void)
{
}

//���ú���
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

//���к���
bool CSocketAcceptThread::RepetitionRun()
{
	//Ч�����
	ASSERT(m_hCompletionPort!=NULL);
	ASSERT(m_pTCPSocketManager!=NULL);

	//���ñ���
	SOCKADDR_IN	SocketAddr;
	CServerSocketItem * pServerSocketItem=NULL;
	SOCKET hConnectSocket=INVALID_SOCKET;
	int nBufferSize=sizeof(SocketAddr);

	try
	{
		//��������
		hConnectSocket=WSAAccept(m_hListenSocket,(SOCKADDR *)&SocketAddr,&nBufferSize,NULL,NULL);
		if (hConnectSocket==INVALID_SOCKET) return false;

		//��ȡ����
		pServerSocketItem=m_pTCPSocketManager->ActiveSocketItem();
		if (pServerSocketItem==NULL) throw TEXT("�������Ӷ���ʧ��");

		//�������
		CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
		pServerSocketItem->Attach(hConnectSocket,SocketAddr.sin_addr.S_un.S_addr);
		CreateIoCompletionPort((HANDLE)hConnectSocket,m_hCompletionPort,(ULONG_PTR)pServerSocketItem,0);
		pServerSocketItem->RecvData();
	}
	catch (...)
	{
		//�������
		ASSERT(pServerSocketItem==NULL);
		if (hConnectSocket!=INVALID_SOCKET)	closesocket(hConnectSocket);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CSocketDetectThread::CSocketDetectThread(void)
{
	m_dwTickCount=0;;
	m_pTCPSocketManager=NULL;
}

//��������
CSocketDetectThread::~CSocketDetectThread(void)
{
}

//���ú���
bool CSocketDetectThread::InitThread(CTCPSocketEngine * pTCPSocketManager)
{
	//Ч�����
	ASSERT(pTCPSocketManager!=NULL);

	//���ñ���
	m_dwTickCount=0L;
	m_pTCPSocketManager=pTCPSocketManager;

	return true;
}

//���к���
bool CSocketDetectThread::RepetitionRun()
{
	//Ч�����
	ASSERT(m_pTCPSocketManager!=NULL);

	//���ü��
	Sleep(500);
	m_dwTickCount += 500L;

	//�������
	if (m_dwTickCount>20000L)
	{
		m_dwTickCount=0L;
		m_pTCPSocketManager->DetectSocket();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CTCPSocketEngine::CTCPSocketEngine(void)
{
	m_bService=false;
	m_bNoDelayMod=false;
	m_wListenPort=0;
	m_dwLastDetect=0;
	m_wMaxSocketItem=0;
	m_hCompletionPort=NULL;
	m_hServerSocket=INVALID_SOCKET;

	//��ʼ�� SOCKET
	WSADATA WSAData;
	WSAStartup(WINSOCK_VERSION,&WSAData);
}

//��������
CTCPSocketEngine::~CTCPSocketEngine(void)
{
	//ֹͣ����
	EndService();

	//�ͷŴ洢����
	for(CServerSocketItemPtrMap::iterator iter = m_StorageSocketItem.begin();
		iter != m_StorageSocketItem.end(); ++iter)
	{
		delete iter->second;
	}

	//���� SOCKET
	WSACleanup();
}

//���ýӿ�
void CTCPSocketEngine::SetSocketEngineSink(CQueueServiceBase * pQueueService)
{
	//״̬�ж�
	if (m_bService==true) 
	{
		return;
	}

	//���ýӿ�
	m_AttemperEvent.SetQueueService(pQueueService);
}

//����NODELAYģʽ
void CTCPSocketEngine::SetNoDelayMod(bool bMode)
{
	m_bNoDelayMod = bMode;
}

//���ö˿�
bool CTCPSocketEngine::SetServicePort(WORD wListenPort)
{
	//Ч��״̬
	if (m_bService==true) 
	{
		return false;
	}

	//�жϲ���
	if (wListenPort==0)
	{
		return false;
	}

	//���ñ���
	m_wListenPort=wListenPort;

	return true;
}

//������Ŀ
bool CTCPSocketEngine::SetMaxSocketItem(WORD wMaxSocketItem)
{
	m_wMaxSocketItem=wMaxSocketItem;
	return true;
}

//��������
bool CTCPSocketEngine::BeginService()
{
	DWORD i = 0;
	//Ч��״̬
	if (m_bService==true) 
	{
		return true;
	}

	//�ж϶˿�
	if (m_wListenPort==0)
	{
		return false;
	}

	//�󶨶���
	m_SendQueueService.SetQueueServiceSink(this);

	try
	{
		//��ȡϵͳ��Ϣ
		SYSTEM_INFO SystemInfo;
		GetSystemInfo(&SystemInfo);
		DWORD dwThreadCount=SystemInfo.dwNumberOfProcessors;

		//������ɶ˿�
		m_hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,SystemInfo.dwNumberOfProcessors);
		if (m_hCompletionPort==NULL) throw TEXT("����������ɶ˿ڴ���ʧ��");

		//��������SOCKET
		struct sockaddr_in SocketAddr;
		memset(&SocketAddr,0,sizeof(SocketAddr));
		SocketAddr.sin_addr.s_addr=INADDR_ANY;
		SocketAddr.sin_family=AF_INET;
		SocketAddr.sin_port=htons(m_wListenPort);
		m_hServerSocket=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);
		if (m_hServerSocket==INVALID_SOCKET) throw TEXT("����������� SOCKET ����ʧ��");
		int iErrorCode=bind(m_hServerSocket,(SOCKADDR*)&SocketAddr,sizeof(SocketAddr));
		if (iErrorCode==SOCKET_ERROR) throw TEXT("������������˿ڱ�ռ�ã��˿ڰ�ʧ��");
		iErrorCode=listen(m_hServerSocket,SOMAXCONN);
		if (iErrorCode==SOCKET_ERROR) throw TEXT("������������˿ڱ�ռ�ã��˿ڼ���ʧ��");

		//�������Ͷ���
		bool bSuccess=m_SendQueueService.BeginService();
		if (bSuccess==false) throw TEXT("�������淢�Ͷ��з�������ʧ��");

		//������д�߳�
		for (i=0;i<dwThreadCount;i++)
		{
			CServerSocketRSThread * pServerSocketRSThread=new CServerSocketRSThread();
			if (pServerSocketRSThread==NULL) throw TEXT("���������д�̷߳��񴴽�ʧ��");
			bSuccess=pServerSocketRSThread->InitThread(m_hCompletionPort);
			if (bSuccess==false) throw TEXT("���������д�̷߳�������ʧ��");
			m_SocketRSThreadArray.push_back(pServerSocketRSThread);
		}

		//����Ӧ���߳�
		bSuccess=m_SocketAcceptThread.InitThread(m_hCompletionPort,m_hServerSocket,this);
		if (bSuccess==false) throw TEXT("����������������̷߳�������");

		//���ж�д�߳�
		for (i=0;i<dwThreadCount;i++)
		{
			CServerSocketRSThread * pServerSocketRSThread=m_SocketRSThreadArray[i];
			ASSERT(pServerSocketRSThread!=NULL);
			bSuccess=pServerSocketRSThread->StartThead();
			if (bSuccess==false) throw TEXT("���������д�̷߳�������ʧ��");
		}

		//�������߳�
		m_SocketDetectThread.InitThread(this);
		bSuccess=m_SocketDetectThread.StartThead();
		if (bSuccess==false) throw TEXT("�����������̷߳�������ʧ��");

		//����Ӧ���߳�
		bSuccess=m_SocketAcceptThread.StartThead();
		if (bSuccess==false) throw TEXT("������������̷߳�������ʧ��");

		//���ñ���
		m_bService=true;
	}
	catch (std::string &strError)
	{
		CLogEngine::Instance().Log(Level_Exception, strError.c_str());
		return false;
	}

	return true;
}

//ֹͣ����
bool CTCPSocketEngine::EndService()
{
	//���ñ���
	m_bService=false;
	m_dwLastDetect=0L;

	//ֹͣ����߳�
	m_SocketDetectThread.StopThread();

	//��ֹӦ���߳�
	if (m_hServerSocket!=INVALID_SOCKET)
	{
		closesocket(m_hServerSocket);
		m_hServerSocket=INVALID_SOCKET;
	}
	m_SocketAcceptThread.StopThread();

	//ֹͣ���Ͷ���
	m_SendQueueService.EndService();

	//�ͷŶ�д�߳�
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

	//�ر���ɶ˿�
	if (m_hCompletionPort!=NULL)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort=NULL;
	}

	//�ر�����
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

//Ӧ����Ϣ
bool CTCPSocketEngine::OnSocketAcceptEvent(CServerSocketItem * pServerSocketItem)
{
	//Ч������
	ASSERT(pServerSocketItem!=NULL);
	if (NULL == pServerSocketItem) return false;

	if(m_bNoDelayMod)
	{
		static const int ndoption = 1;
		setsockopt(pServerSocketItem->GetHandle(), IPPROTO_TCP, TCP_NODELAY, (const char*)&ndoption, sizeof(ndoption));
	}

	//Ͷ����Ϣ
	if (m_bService==false) return false;
	m_AttemperEvent.PostSocketAcceptEvent(pServerSocketItem->GetSocketID(),pServerSocketItem->GetClientAddr());

	return true;
}

//�����ȡ��Ϣ
bool CTCPSocketEngine::OnSocketReadEvent(DWORD dwCommand, void * pBuffer, WORD wDataSize, CServerSocketItem * pServerSocketItem)
{
	//Ч������
	ASSERT(pServerSocketItem!=NULL);
	if (NULL == pServerSocketItem) return false;

	//Ч��״̬
	if (m_bService==false) return false;
	m_AttemperEvent.PostSocketReadEvent(pServerSocketItem->GetSocketID(),dwCommand,pBuffer,wDataSize);

	return true;
}

//����ر���Ϣ
bool CTCPSocketEngine::OnSocketCloseEvent(CServerSocketItem * pServerSocketItem)
{
	//Ч�����
	ASSERT(pServerSocketItem!=NULL);
	if (NULL == pServerSocketItem) return false;

	try
	{
		//Ч��״̬
		if (m_bService==false) return false;

		//����ʱ��
		DWORD dwClientAddr=pServerSocketItem->GetClientAddr();
		DWORD dwConnectTime=pServerSocketItem->GetConnectTime();
		m_AttemperEvent.PostSocketCloseEvent(pServerSocketItem->GetSocketID(),dwClientAddr,dwConnectTime);

		//�ͷ�����
		FreeSocketItem(pServerSocketItem);
	}
	catch (...) {}

	return true;
}

//֪ͨ�ص����������Ͷ����̵߳��ã�
void CTCPSocketEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	switch (wIdentifier)
	{
	case QUEUE_SEND_REQUEST:		//��������
		{
			//Ч������
			tagSendDataRequest * pSendDataRequest=(tagSendDataRequest *)pBuffer;
			ASSERT(wDataSize>=(sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuf)));
			ASSERT(wDataSize==(pSendDataRequest->wDataSize+sizeof(tagSendDataRequest)-sizeof(pSendDataRequest->cbSendBuf)));

			//��������
			if (pSendDataRequest->wIndex==INDEX_ALL_SOCKET)
			{
				//��ȡ���
				CThreadLockHandle ItemLockedHandle(&m_ItemLocked);
				m_TempSocketItem.clear();
				m_TempSocketItem.insert(m_ActiveSocketItem.begin(), m_ActiveSocketItem.end());
				ItemLockedHandle.UnLock();

				//ѭ����������
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
				//�����
				CServerSocketItem * pServerSocketItem=EnumSocketItem(pSendDataRequest->wIndex);
				CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
				pServerSocketItem->SendData(pSendDataRequest->dwCommand,pSendDataRequest->cbSendBuf,pSendDataRequest->wDataSize,pServerSocketItem->GetRountID());
			}

			break;
		}
	case QUEUE_SAFE_CLOSE:		//��ȫ�ر�
		{
			//Ч������
			ASSERT(wDataSize==sizeof(tagSafeCloseSocket));
			tagSafeCloseSocket * pSafeCloseSocket=(tagSafeCloseSocket *)pBuffer;

			//��ȫ�ر�
			CServerSocketItem * pServerSocketItem=EnumSocketItem(pSafeCloseSocket->wIndex);
			CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
			pServerSocketItem->ShutDownSocket(pSafeCloseSocket->wRountID);

			break;
		}
	case QUEUE_DETECT_SOCKET:	//�������
		{
			//��ȡ���
			CThreadLockHandle ItemLockedHandle(&m_ItemLocked);
			m_TempSocketItem.clear();
			m_TempSocketItem.insert(m_ActiveSocketItem.begin(), m_ActiveSocketItem.end());
			ItemLockedHandle.UnLock();

			//��������
			CMD_KN_DetectSocket DetectSocket;
			ZeroMemory(&DetectSocket,sizeof(DetectSocket));

			//��������
			WORD wRountID=0;
			DWORD dwNowTickCount=GetTickCount();
			DWORD dwBreakTickCount=__max(dwNowTickCount-m_dwLastDetect,TIME_BREAK_READY);

			//���ñ���
			m_dwLastDetect=GetTickCount();

			//�������
			CServerSocketItem * pServerSocketItem=NULL;
			for (CServerSocketItemPtrSet::iterator iter = m_TempSocketItem.begin();
				iter != m_TempSocketItem.end(); ++iter)
			{
				//��������
				pServerSocketItem=*iter;
				DWORD dwRecvTickCount=pServerSocketItem->GetRecvTickCount();
				CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());

				//������
				if (dwRecvTickCount>=dwNowTickCount) continue;

				//�������
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

				//��������
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

//��ȡ���ж���
CServerSocketItem * CTCPSocketEngine::ActiveSocketItem()
{
	CThreadLockHandle ItemLockedHandle(&m_ItemLocked,true);

	//��ȡ���ж���
	CServerSocketItem * pServerSocketItem=NULL;
	if (m_FreeSocketItem.size()>0)
	{
		pServerSocketItem=*m_FreeSocketItem.begin();
		ASSERT(pServerSocketItem!=NULL);
		m_FreeSocketItem.erase(m_FreeSocketItem.begin());
		m_ActiveSocketItem.insert(pServerSocketItem);
	}

	//�����¶���
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

//��ȡ���Ӷ���
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

//�ͷ����Ӷ���
bool CTCPSocketEngine::FreeSocketItem(CServerSocketItem * pServerSocketItem)
{
	//Ч�����
	ASSERT(pServerSocketItem!=NULL);

	//�ͷŶ���
	CThreadLockHandle ItemLockedHandle(&m_ItemLocked,true);

	CServerSocketItemPtrSet::iterator iter = m_ActiveSocketItem.find(pServerSocketItem);
	if(iter != m_ActiveSocketItem.end())
	{
		m_FreeSocketItem.insert(pServerSocketItem);
		m_ActiveSocketItem.erase(iter);
		return true;
	}

	//�ͷ�ʧ��
	ASSERT(FALSE);
	return false;
}

//�������
bool CTCPSocketEngine::DetectSocket()
{
	return m_SendQueueService.AddToQueue(QUEUE_DETECT_SOCKET,NULL,0);
}

//���ͺ���
bool CTCPSocketEngine::SendData(DWORD dwSocketID, DWORD dwCommand, void * pData, WORD wDataSize)
{
	//Ч��״̬
	ASSERT(m_bService==true);
	if (m_bService==false) return false;

	//Ч������
	ASSERT((wDataSize+sizeof(CMD_Head))<=SOCKET_PACKET);
	if ((wDataSize+sizeof(CMD_Head))>SOCKET_PACKET) return false;

	//��������
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

	//���뷢�Ͷ���
	WORD wSendSize=sizeof(SendRequest)-sizeof(SendRequest.cbSendBuf)+wDataSize;
	return m_SendQueueService.AddToQueue(QUEUE_SEND_REQUEST,&SendRequest,wSendSize);
}

//��������
bool CTCPSocketEngine::SendDataBatch(DWORD dwCommand, void * pData, WORD wDataSize)
{
	//Ч��״̬
	if (m_bService==false) return false;

	//Ч������
	ASSERT((wDataSize+sizeof(CMD_Head))<=SOCKET_PACKET);
	if ((wDataSize+sizeof(CMD_Head))>SOCKET_PACKET) return false;

	//��������
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

	//���뷢�Ͷ���
	WORD wSendSize=sizeof(SendRequest)-sizeof(SendRequest.cbSendBuf)+wDataSize;
	return m_SendQueueService.AddToQueue(QUEUE_SEND_REQUEST,&SendRequest,wSendSize);
}

//�ر�����
bool CTCPSocketEngine::CloseSocket(DWORD dwSocketID)
{
	CServerSocketItem * pServerSocketItem=EnumSocketItem(GetIndexFromeSocketID(dwSocketID));
	if (pServerSocketItem==NULL) return false;
	CThreadLockHandle SocketLockHandle(pServerSocketItem->GetSignedLock());
	return pServerSocketItem->CloseSocket(GetRoundIDFromSocketID(dwSocketID));
}

//���ùر�
bool CTCPSocketEngine::ShutDownSocket(DWORD dwSocketID)
{
	tagSafeCloseSocket SafeCloseSocket;
	SafeCloseSocket.wIndex=GetIndexFromeSocketID(dwSocketID);
	SafeCloseSocket.wRountID=GetRoundIDFromSocketID(dwSocketID);
	return m_SendQueueService.AddToQueue(QUEUE_SAFE_CLOSE,&SafeCloseSocket,sizeof(SafeCloseSocket));
}

//////////////////////////////////////////////////////////////////////////