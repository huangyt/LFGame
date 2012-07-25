#include "QueueService.h"
#include "DataStorage.h"

//////////////////////////////////////////////////////////////////////////

//���캯��
CQueueServiceThread::CQueueServiceThread(void)
{
	m_hCompletionPort=NULL;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));
}

//��������
CQueueServiceThread::~CQueueServiceThread(void)
{
}

//���ú���
bool CQueueServiceThread::InitThread(HANDLE hCompletionPort)
{
	//Ч�����
	ASSERT(IsRuning()==false);
	ASSERT(m_hCompletionPort==NULL);

	//���ñ���
	m_hCompletionPort=hCompletionPort;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));

	return true;
}

//ȡ������
bool CQueueServiceThread::UnInitThread()
{
	//Ч�����
	ASSERT(IsRuning()==false);

	//���ñ���
	m_hCompletionPort=NULL;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));

	return true;
}

//���к���
bool CQueueServiceThread::RepetitionRun()
{
	//Ч�����
	ASSERT(m_hCompletionPort!=NULL);

	//��������
	DWORD dwThancferred=0;
	OVERLAPPED * pOverLapped=NULL;
	CQueueService * pQueueService=NULL;

	//�ȴ���ɶ˿�
	if (GetQueuedCompletionStatus(m_hCompletionPort,&dwThancferred,(PULONG_PTR)&pQueueService,&pOverLapped,INFINITE))
	{
		//�ж��˳�
		if (pQueueService==NULL) return false;

		//��ȡ����
		tagDataHead DataHead;
		bool bSuccess=pQueueService->GetData(DataHead,m_cbBuffer,sizeof(m_cbBuffer));
		ASSERT(bSuccess==true);

		//��������
		if (bSuccess==true) pQueueService->OnQueueServiceThread(DataHead,m_cbBuffer,DataHead.wDataSize);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CQueueService::CQueueService(void)
{
	m_bService=false;
	m_hCompletionPort=NULL;
	m_pQueueServiceSink=NULL;
	m_lUnComplete=0;
}

//��������
CQueueService::~CQueueService(void)
{
	//ֹͣ����
	EndService();

	return;
}

//���ýӿ�
void CQueueService::SetQueueServiceSink(CQueueServiceSink *pQueueServiceSink)
{
	ASSERT(pQueueServiceSink!=NULL);
	m_pQueueServiceSink = pQueueServiceSink;
}

//������Ϣ
bool CQueueService::GetBurthenInfo(tagBurthenInfo & BurthenInfo)
{
	CThreadLockHandle LockHandle(&m_ThreadLock);
	return m_DataStorage.GetBurthenInfo(BurthenInfo);
}

//��������
bool CQueueService::AddToQueue(WORD wIdentifier, void * const pBuffer, WORD wDataSize)
{
	CThreadLockHandle LockHandle(&m_ThreadLock);
	m_DataStorage.AddData(wIdentifier,pBuffer,wDataSize);
	PostQueuedCompletionStatus(m_hCompletionPort,wDataSize,(ULONG_PTR)this,NULL);
	m_lUnComplete++;
	return true;
}

LONG CQueueService::GetUnCompleteCount()
{
	CThreadLockHandle LockHandle(&m_ThreadLock);
	return m_lUnComplete;
}

//��ʼ����
bool CQueueService::BeginService()
{
	//Ч�����
	ASSERT(m_bService==false);
	ASSERT(m_hCompletionPort==NULL);
	ASSERT(m_pQueueServiceSink!=NULL);

	//������ɶ˿�
	m_hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,1);
	if (m_hCompletionPort==NULL) throw TEXT("���ж�����ɶ˿ڴ���ʧ��");

	//�����߳�
	if (m_QueueServiceThread.InitThread(m_hCompletionPort)==false) throw TEXT("���ж����̳߳�ʼ��ʧ��");
	if (m_QueueServiceThread.StartThead()==false) throw TEXT("���ж����߳�����ʧ��");

	//���ò���
	m_bService=true;

	return true;
}

//ֹͣ����
bool CQueueService::EndService()
{
	//���ñ���
	m_bService=false;

	//ֹͣ�߳�
	if (m_hCompletionPort!=NULL) PostQueuedCompletionStatus(m_hCompletionPort,0,NULL,NULL);
	m_QueueServiceThread.StopThread();
	m_QueueServiceThread.UnInitThread();

	//�ر���ɶ˿�
	if (m_hCompletionPort!=NULL)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort=NULL;
	}

	//��������
	m_DataStorage.RemoveData(false);

	return true;
}

//��ȡ����
bool CQueueService::GetData(tagDataHead & DataHead, void * pBuffer, WORD wBufferSize)
{
	CThreadLockHandle LockHandle(&m_ThreadLock);
	m_lUnComplete--;
	return m_DataStorage.GetData(DataHead,pBuffer,wBufferSize);
}

//������Ϣ
void CQueueService::OnQueueServiceThread(const tagDataHead & DataHead, void * pBuffer, WORD wDataSize)
{
	ASSERT(m_pQueueServiceSink!=NULL);
	try	
	{
		m_pQueueServiceSink->OnQueueServiceSink(DataHead.wIdentifier,pBuffer,DataHead.wDataSize); 
	}
	catch (...) {}
	return;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CQueueServiceEvent::CQueueServiceEvent()
:m_pQueueService(NULL)
{

}

//��������
CQueueServiceEvent::~CQueueServiceEvent()
{

}

//���ýӿ�
void CQueueServiceEvent::SetQueueService(CQueueServiceBase *pQueueService)
{
	ASSERT(pQueueService!=NULL);
	m_pQueueService = pQueueService;
}

//��ʱ���¼�
bool CQueueServiceEvent::PostTimerEvent(WORD wTimerID, WPARAM wBindParam)
{
	//Ч�����
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;

	//��������
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//Ͷ����Ϣ
	NTY_TimerEvent * pTimerEvent=(NTY_TimerEvent *)m_cbBuffer;
	pTimerEvent->wTimerID=wTimerID;
	pTimerEvent->wBindParam=wBindParam;
	m_pQueueService->AddToQueue(EVENT_TIMER,m_cbBuffer,sizeof(NTY_TimerEvent));

	return true;
}

//���ݿ��¼�
bool CQueueServiceEvent::PostDataBaseEvent(WORD wRequestID, DWORD dwSocketID, const void * pDataBuffer, WORD wDataSize)
{
	//Ч�����
	ASSERT(m_pQueueService!=NULL);
	ASSERT((wDataSize+sizeof(NTY_DataBaseEvent))<=MAX_QUEUE_PACKET);
	if (m_pQueueService==NULL) return false;
	if ((wDataSize+sizeof(NTY_DataBaseEvent))>MAX_QUEUE_PACKET) return false;

	//��������
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//Ͷ����Ϣ
	NTY_DataBaseEvent * pDataBaseEvent=(NTY_DataBaseEvent *)m_cbBuffer;
	pDataBaseEvent->dwSocketID=dwSocketID;
	pDataBaseEvent->wRequestID=wRequestID;
	pDataBaseEvent->wDataSize=wDataSize;
	pDataBaseEvent->pDataBuffer=NULL;
	if (wDataSize>0)
	{
		ASSERT(pDataBuffer!=NULL);
		CopyMemory(m_cbBuffer+sizeof(NTY_DataBaseEvent),pDataBuffer,wDataSize);
	}
	m_pQueueService->AddToQueue(EVENT_DATABASE,m_cbBuffer,sizeof(NTY_DataBaseEvent)+wDataSize);

	return true;
}

//����Ӧ���¼�
bool CQueueServiceEvent::PostSocketAcceptEvent(DWORD dwSocketID, DWORD dwClientIP)
{
	//Ч�����
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;

	//��������
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//Ͷ����Ϣ
	NTY_SocketAcceptEvent * pSocketAcceptEvent=(NTY_SocketAcceptEvent *)m_cbBuffer;
	pSocketAcceptEvent->dwSocketID=dwSocketID;
	pSocketAcceptEvent->dwClientIP=dwClientIP;
	m_pQueueService->AddToQueue(EVENT_SOCKET_ACCEPT,m_cbBuffer,sizeof(NTY_SocketAcceptEvent));

	return true;
}

//�����ȡ�¼�
bool CQueueServiceEvent::PostSocketReadEvent(DWORD dwSocketID, DWORD dwCommand, const void * pDataBuffer, WORD wDataSize)
{
	//Ч�����
	ASSERT(m_pQueueService!=NULL);
	ASSERT((wDataSize+sizeof(NTY_SocketReadEvent))<=MAX_QUEUE_PACKET);
	if (m_pQueueService==NULL) return false;
	if ((wDataSize+sizeof(NTY_SocketReadEvent))>MAX_QUEUE_PACKET) return false;

	//��������
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//Ͷ����Ϣ
	NTY_SocketReadEvent * pSocketReadEvent=(NTY_SocketReadEvent *)m_cbBuffer;
	pSocketReadEvent->dwSocketID=dwSocketID;
	pSocketReadEvent->dwCommand=dwCommand;
	pSocketReadEvent->wDataSize=wDataSize;
	pSocketReadEvent->pDataBuffer=NULL;
	if (wDataSize>0)
	{
		ASSERT(pDataBuffer!=NULL);
		CopyMemory(m_cbBuffer+sizeof(NTY_SocketReadEvent),pDataBuffer,wDataSize);
	}
	m_pQueueService->AddToQueue(EVENT_SOCKET_READ,m_cbBuffer,sizeof(NTY_SocketReadEvent)+wDataSize);

	return false;
}

//����ر��¼�
bool CQueueServiceEvent::PostSocketCloseEvent(DWORD dwSocketID, DWORD dwClientIP, DWORD dwConnectSecond)
{
	//Ч�����
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;

	//��������
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//Ͷ����Ϣ
	NTY_SocketCloseEvent * pSocketCloseEvent=(NTY_SocketCloseEvent *)m_cbBuffer;
	pSocketCloseEvent->dwSocketID=dwSocketID;
	pSocketCloseEvent->dwClientIP=dwClientIP;
	pSocketCloseEvent->dwConnectSecond=dwConnectSecond;
	m_pQueueService->AddToQueue(EVENT_SOCKET_CLOSE,m_cbBuffer,sizeof(NTY_SocketCloseEvent));

	return true;
}

LONG CQueueServiceEvent::GetUnCompleteCount()
{
	//Ч�����
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;
	//��������
	CThreadLockHandle BufferLockHandle(&m_BufferLock);
	return m_pQueueService->GetUnCompleteCount();
}

//////////////////////////////////////////////////////////////////////////
