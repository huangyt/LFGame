#include "TcpSocket.h"
#include "FormatString.h"
#include "BigNumber.h"

//////////////////////////////////////////////////////////////////////////

//���캯��
CSocketThread::CSocketThread()
{
	m_pTcpSocket = NULL;
}

//��������
CSocketThread::~CSocketThread()
{
}

//���ú���
bool CSocketThread::InitThread(CTcpSocket * pTcpSocket)
{
	if(pTcpSocket==NULL) return false;

	m_pTcpSocket = pTcpSocket;

	return true;
}

//���к���
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

	//��ʼ�� SOCKET
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

	//���� SOCKET
	WSACleanup();
}

//���ýӿ�
void CTcpSocket::SetSocketSink(CTcpSocketSink *pTcpSocketSink)
{
	ASSERT(pTcpSocketSink!=NULL);
	m_pTcpSocketSink = pTcpSocketSink;
}

//����NODELAYģʽ
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

//���ӷ�����
bool CTcpSocket::Connect(DWORD dwServerIP, WORD wPort)
{
	//Ч�����
	ASSERT(m_hSocket==INVALID_SOCKET);
	ASSERT(m_SocketState==SocketState_NoConnect);

	//Ч��״̬
	if (m_hSocket!=INVALID_SOCKET) throw TEXT("���� SOCKET ����Ѿ�����");
	if (m_SocketState!=SocketState_NoConnect) throw TEXT("����״̬���ǵȴ�����״̬");
	if (dwServerIP==INADDR_NONE) throw TEXT("Ŀ���������ַ��ʽ����ȷ��������ٴγ��ԣ�");

	//���ò���
	m_wRecvSize=0;
	m_dwSendTickCount=GetTickCount()/1000L;
	m_dwRecvTickCount=GetTickCount()/1000L;

	try
	{
		//���� SOCKET
		m_hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if (m_hSocket==INVALID_SOCKET) throw TEXT("SOCKET ����ʧ��");

		//��д��������ַ
		SOCKADDR_IN SocketAddr;
		memset(&SocketAddr,0,sizeof(SocketAddr));
		SocketAddr.sin_family=AF_INET;
		SocketAddr.sin_port=htons(wPort);
		SocketAddr.sin_addr.S_un.S_addr=dwServerIP;

		//���ӷ�����
		int iErrorCode=0;
		WSASetLastError(0);

		//����SOCKET�¼�
		iErrorCode = WSAEventSelect(m_hSocket, m_hSocketEvent, FD_CONNECT|FD_READ|FD_CLOSE);
		if (iErrorCode==SOCKET_ERROR) throw TEXT("�������¼�����");

		//���ӷ�����
		iErrorCode=connect(m_hSocket,(SOCKADDR *)&SocketAddr,sizeof(SocketAddr));
		if (iErrorCode==SOCKET_ERROR)
		{
			iErrorCode=WSAGetLastError();
			if (iErrorCode!=WSAEWOULDBLOCK)
			{
				std::wstring strError = std::move(wstrFormat(L"���ӷ������󣬴������ [ %d ]", iErrorCode));
				throw strError;
			}
		}

		//���ñ���
		m_SocketState=SocketState_Connecting;

		//����������Ϣ�߳�
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
		throw TEXT("���Ӳ���δ֪�쳣����");
	}
	return false;
}

//���ӷ�����
bool CTcpSocket::Connect(const std::wstring& strServerIP, WORD wPort)
{
	//Ч������
	ASSERT(wPort!=0);
	ASSERT(strServerIP.empty()==false);
	if (wPort==0) return false;
	if (strServerIP.empty()) return false;
	return Connect(TranslateAddr(strServerIP),wPort);
}

//���ͺ���
bool CTcpSocket::SendData(DWORD dwCommand, void * pData, WORD wDataSize)
{
	//Ч��״̬
	if (m_hSocket==INVALID_SOCKET) return false;
	if (m_SocketState!=SocketState_Connected) return false;

	//Ч���С
	ASSERT(wDataSize<=SOCKET_PACKET);
	if (wDataSize>SOCKET_PACKET) return false;

	//��������
	BYTE cbDataBuffer[SOCKET_BUFFER];
	CMD_Head * pHead=(CMD_Head *)cbDataBuffer;
	pHead->dwCommand=dwCommand;
	if (wDataSize>0)
	{
		ASSERT(pData!=NULL);
		CopyMemory(pHead+1,pData,wDataSize);
	}

	//��������
	WORD wSendSize=EncryptBuffer(cbDataBuffer,sizeof(CMD_Head)+wDataSize,sizeof(cbDataBuffer));

	//��������
	return SendBuffer(cbDataBuffer,wSendSize);
}

//�ر�����
bool CTcpSocket::CloseSocket(bool bNotify)
{
	//�ر�����
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

	//�ָ�����
	m_wRecvSize=0;
	m_dwSendTickCount=0;
	m_dwRecvTickCount=0;
	m_dwSendPacketCount=0;
	m_dwRecvPacketCount=0;
	m_bCloseByServer=false;

	return true;
}

//��������
bool CTcpSocket::SendBuffer(void * pBuffer, WORD wSendSize)
{
	//Ч�����
	ASSERT(wSendSize!=0);
	ASSERT(pBuffer!=NULL);

	//��������
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

//���ͷ�������ַ
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

//���ʹ���
std::wstring CTcpSocket::GetConnectError(int iErrorCode,std::wstring& strBuffer)
{
	//���ʹ���
	switch (iErrorCode)
	{
	case 0:					//û�д���
		{
			strBuffer = L"����ִ�гɹ���";
			break;
		}
	case WSAEADDRNOTAVAIL:	//��ַ��ʽ����
		{
			strBuffer = L"Ŀ���������ַ��ʽ����ȷ��������ٴγ��ԣ�";
			break;
		}
	case WSAECONNREFUSED:	//������û������
		{
			strBuffer = L"Ŀ���������æ����û��������";
			break;
		}
	case WSAETIMEDOUT:		//���ӳ�ʱ
		{
			strBuffer = L"���ӳ�ʱ��������Ŀ������������ڻ��߷�������ַ��ʽ����ȷ��";
			break;
		}
	case WSAEHOSTUNREACH:
		{
			strBuffer = L"��������ʧ�ܣ������Ƿ��Ѿ��ɹ����ź����� Internet ��";
			break;
		}
	default:				//Ĭ�ϴ���
		{
			strBuffer = std::move(wstrFormat(L"���Ӵ���ţ�%ld����ϸ������Ϣ��ο����������ֲᣡ",iErrorCode));
			break;
		}
	}
	return strBuffer;
}

//��������
WORD CTcpSocket::EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize)
{
	//Ч�����
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(wBufferSize>=(wDataSize+2*sizeof(DWORD)));
	ASSERT(wDataSize<=(sizeof(CMD_Head)+SOCKET_PACKET));

	//Ч����
	BYTE cbCheckCode=0;
	for (WORD i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
	}

	//��д��Ϣͷ
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	pHead->CmdInfo.cbCheckCode=~cbCheckCode+1;
	pHead->CmdInfo.wDataSize=wDataSize;
	pHead->CmdInfo.cbMessageVer=SOCKET_VER;

	//��������-ֻ����Command
	m_AuthClientCrypt.EncryptSend(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//���ñ���
	m_dwSendPacketCount++;

	return wDataSize;
}

//��������
WORD CTcpSocket::CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize)
{
	//Ч�����
	ASSERT(m_dwSendPacketCount>0);
	ASSERT(wDataSize>=sizeof(CMD_Head));
	ASSERT(((CMD_Head *)pcbDataBuffer)->CmdInfo.wDataSize==wDataSize);

	//��������
	m_AuthClientCrypt.DecryptRecv(pcbDataBuffer+sizeof(CMD_Info), wDataSize - sizeof(CMD_Info));

	//Ч����
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	BYTE cbCheckCode=pHead->CmdInfo.cbCheckCode;
	for (int i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
	}
	if (cbCheckCode!=0) throw TEXT("���ݰ�Ч�������");

	return wDataSize;
}

//��������
LRESULT CTcpSocket::OnSocketNotifyConnect(int iErrorCode)
{
	//�ж�״̬
	if (iErrorCode==0) m_SocketState=SocketState_Connected;
	else CloseSocket(false);

	//����֪ͨ
	std::wstring strErrorDesc;
	GetConnectError(iErrorCode,strErrorDesc);
	m_pTcpSocketSink->OnSocketConnect(iErrorCode,strErrorDesc,this);

	return 1;
}

//�����ȡ
LRESULT CTcpSocket::OnSocketNotifyRead(int iErrorCode)
{
	try
	{
		//��ȡ����
		int iRetCode=recv(m_hSocket,(char *)m_cbRecvBuf+m_wRecvSize,sizeof(m_cbRecvBuf)-m_wRecvSize,0);
		if (iRetCode==SOCKET_ERROR) throw TEXT("�������ӹرգ���ȡ����ʧ��");
		ASSERT(m_dwSendPacketCount>0);
		m_wRecvSize+=iRetCode;
		m_dwRecvTickCount=GetTickCount()/1000L;

		//��������
		WORD wPacketSize=0;
		BYTE cbDataBuffer[SOCKET_PACKET+sizeof(CMD_Head)];
		CMD_Head * pHead=(CMD_Head *)m_cbRecvBuf;

		while (m_wRecvSize>=sizeof(CMD_Head))
		{
			//Ч�����
			wPacketSize=pHead->CmdInfo.wDataSize;
			ASSERT(pHead->CmdInfo.cbMessageVer==SOCKET_VER);
			ASSERT(wPacketSize<=(SOCKET_PACKET+sizeof(CMD_Head)));
			if (pHead->CmdInfo.cbMessageVer!=SOCKET_VER) throw TEXT("���ݰ��汾����");
			if (wPacketSize>(SOCKET_PACKET+sizeof(CMD_Head))) throw TEXT("���ݰ�̫��");
			if (m_wRecvSize<wPacketSize) return 1;

			//��������
			m_dwRecvPacketCount++;
			CopyMemory(cbDataBuffer,m_cbRecvBuf,wPacketSize);
			m_wRecvSize-=wPacketSize;
			MoveMemory(m_cbRecvBuf,m_cbRecvBuf+wPacketSize,m_wRecvSize);

			//��������
			WORD wRealySize=CrevasseBuffer(cbDataBuffer,wPacketSize);
			ASSERT(wRealySize>=sizeof(CMD_Head));

			//��������
			WORD wDataSize=wRealySize-sizeof(CMD_Head);
			void * pDataBuffer=cbDataBuffer+sizeof(CMD_Head);
			DWORD dwCommand=((CMD_Head *)cbDataBuffer)->dwCommand;

			//������
			if (dwCommand==CMD_DETECT_SOCKET)
			{
						//��������
				SendData(CMD_DETECT_SOCKET,pDataBuffer,wDataSize);
				continue;
			}

			//��������
			bool bSuccess=m_pTcpSocketSink->OnSocketRead(dwCommand,pDataBuffer,wDataSize,this);
			if (bSuccess==false) throw TEXT("�������ݰ�����ʧ��");
		};
	}
	catch (...) 
	{ 
		CloseSocket(true); 
	}

	return 1;
}

//����ر�
LRESULT CTcpSocket::OnSocketNotifyClose(int iErrorCode)
{
	m_bCloseByServer=true;
	CloseSocket(true);
	return 1;
}
//////////////////////////////////////////////////////////////////////////