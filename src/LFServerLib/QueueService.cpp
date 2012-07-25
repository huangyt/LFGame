#include "QueueService.h"
#include "DataStorage.h"

//////////////////////////////////////////////////////////////////////////

//构造函数
CQueueServiceThread::CQueueServiceThread(void)
{
	m_hCompletionPort=NULL;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));
}

//析构函数
CQueueServiceThread::~CQueueServiceThread(void)
{
}

//配置函数
bool CQueueServiceThread::InitThread(HANDLE hCompletionPort)
{
	//效验参数
	ASSERT(IsRuning()==false);
	ASSERT(m_hCompletionPort==NULL);

	//设置变量
	m_hCompletionPort=hCompletionPort;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));

	return true;
}

//取消配置
bool CQueueServiceThread::UnInitThread()
{
	//效验参数
	ASSERT(IsRuning()==false);

	//设置变量
	m_hCompletionPort=NULL;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));

	return true;
}

//运行函数
bool CQueueServiceThread::RepetitionRun()
{
	//效验参数
	ASSERT(m_hCompletionPort!=NULL);

	//变量定义
	DWORD dwThancferred=0;
	OVERLAPPED * pOverLapped=NULL;
	CQueueService * pQueueService=NULL;

	//等待完成端口
	if (GetQueuedCompletionStatus(m_hCompletionPort,&dwThancferred,(PULONG_PTR)&pQueueService,&pOverLapped,INFINITE))
	{
		//判断退出
		if (pQueueService==NULL) return false;

		//获取数据
		tagDataHead DataHead;
		bool bSuccess=pQueueService->GetData(DataHead,m_cbBuffer,sizeof(m_cbBuffer));
		ASSERT(bSuccess==true);

		//处理数据
		if (bSuccess==true) pQueueService->OnQueueServiceThread(DataHead,m_cbBuffer,DataHead.wDataSize);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CQueueService::CQueueService(void)
{
	m_bService=false;
	m_hCompletionPort=NULL;
	m_pQueueServiceSink=NULL;
	m_lUnComplete=0;
}

//析构函数
CQueueService::~CQueueService(void)
{
	//停止服务
	EndService();

	return;
}

//设置接口
void CQueueService::SetQueueServiceSink(CQueueServiceSink *pQueueServiceSink)
{
	ASSERT(pQueueServiceSink!=NULL);
	m_pQueueServiceSink = pQueueServiceSink;
}

//负荷信息
bool CQueueService::GetBurthenInfo(tagBurthenInfo & BurthenInfo)
{
	CThreadLockHandle LockHandle(&m_ThreadLock);
	return m_DataStorage.GetBurthenInfo(BurthenInfo);
}

//加入数据
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

//开始服务
bool CQueueService::BeginService()
{
	//效验参数
	ASSERT(m_bService==false);
	ASSERT(m_hCompletionPort==NULL);
	ASSERT(m_pQueueServiceSink!=NULL);

	//建立完成端口
	m_hCompletionPort=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,1);
	if (m_hCompletionPort==NULL) throw TEXT("队列对象完成端口创建失败");

	//启动线程
	if (m_QueueServiceThread.InitThread(m_hCompletionPort)==false) throw TEXT("队列对象线程初始化失败");
	if (m_QueueServiceThread.StartThead()==false) throw TEXT("队列对象线程启动失败");

	//设置参数
	m_bService=true;

	return true;
}

//停止服务
bool CQueueService::EndService()
{
	//设置变量
	m_bService=false;

	//停止线程
	if (m_hCompletionPort!=NULL) PostQueuedCompletionStatus(m_hCompletionPort,0,NULL,NULL);
	m_QueueServiceThread.StopThread();
	m_QueueServiceThread.UnInitThread();

	//关闭完成端口
	if (m_hCompletionPort!=NULL)
	{
		CloseHandle(m_hCompletionPort);
		m_hCompletionPort=NULL;
	}

	//设置数据
	m_DataStorage.RemoveData(false);

	return true;
}

//提取数据
bool CQueueService::GetData(tagDataHead & DataHead, void * pBuffer, WORD wBufferSize)
{
	CThreadLockHandle LockHandle(&m_ThreadLock);
	m_lUnComplete--;
	return m_DataStorage.GetData(DataHead,pBuffer,wBufferSize);
}

//数据消息
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

//构造函数
CQueueServiceEvent::CQueueServiceEvent()
:m_pQueueService(NULL)
{

}

//析构函数
CQueueServiceEvent::~CQueueServiceEvent()
{

}

//设置接口
void CQueueServiceEvent::SetQueueService(CQueueServiceBase *pQueueService)
{
	ASSERT(pQueueService!=NULL);
	m_pQueueService = pQueueService;
}

//定时器事件
bool CQueueServiceEvent::PostTimerEvent(WORD wTimerID, WPARAM wBindParam)
{
	//效验参数
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;

	//缓冲锁定
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//投递消息
	NTY_TimerEvent * pTimerEvent=(NTY_TimerEvent *)m_cbBuffer;
	pTimerEvent->wTimerID=wTimerID;
	pTimerEvent->wBindParam=wBindParam;
	m_pQueueService->AddToQueue(EVENT_TIMER,m_cbBuffer,sizeof(NTY_TimerEvent));

	return true;
}

//数据库事件
bool CQueueServiceEvent::PostDataBaseEvent(WORD wRequestID, DWORD dwSocketID, const void * pDataBuffer, WORD wDataSize)
{
	//效验参数
	ASSERT(m_pQueueService!=NULL);
	ASSERT((wDataSize+sizeof(NTY_DataBaseEvent))<=MAX_QUEUE_PACKET);
	if (m_pQueueService==NULL) return false;
	if ((wDataSize+sizeof(NTY_DataBaseEvent))>MAX_QUEUE_PACKET) return false;

	//缓冲锁定
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//投递消息
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

//网络应答事件
bool CQueueServiceEvent::PostSocketAcceptEvent(DWORD dwSocketID, DWORD dwClientIP)
{
	//效验参数
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;

	//缓冲锁定
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//投递消息
	NTY_SocketAcceptEvent * pSocketAcceptEvent=(NTY_SocketAcceptEvent *)m_cbBuffer;
	pSocketAcceptEvent->dwSocketID=dwSocketID;
	pSocketAcceptEvent->dwClientIP=dwClientIP;
	m_pQueueService->AddToQueue(EVENT_SOCKET_ACCEPT,m_cbBuffer,sizeof(NTY_SocketAcceptEvent));

	return true;
}

//网络读取事件
bool CQueueServiceEvent::PostSocketReadEvent(DWORD dwSocketID, DWORD dwCommand, const void * pDataBuffer, WORD wDataSize)
{
	//效验参数
	ASSERT(m_pQueueService!=NULL);
	ASSERT((wDataSize+sizeof(NTY_SocketReadEvent))<=MAX_QUEUE_PACKET);
	if (m_pQueueService==NULL) return false;
	if ((wDataSize+sizeof(NTY_SocketReadEvent))>MAX_QUEUE_PACKET) return false;

	//缓冲锁定
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//投递消息
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

//网络关闭事件
bool CQueueServiceEvent::PostSocketCloseEvent(DWORD dwSocketID, DWORD dwClientIP, DWORD dwConnectSecond)
{
	//效验参数
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;

	//缓冲锁定
	CThreadLockHandle BufferLockHandle(&m_BufferLock);

	//投递消息
	NTY_SocketCloseEvent * pSocketCloseEvent=(NTY_SocketCloseEvent *)m_cbBuffer;
	pSocketCloseEvent->dwSocketID=dwSocketID;
	pSocketCloseEvent->dwClientIP=dwClientIP;
	pSocketCloseEvent->dwConnectSecond=dwConnectSecond;
	m_pQueueService->AddToQueue(EVENT_SOCKET_CLOSE,m_cbBuffer,sizeof(NTY_SocketCloseEvent));

	return true;
}

LONG CQueueServiceEvent::GetUnCompleteCount()
{
	//效验参数
	ASSERT(m_pQueueService!=NULL);
	if (m_pQueueService==NULL) return false;
	//缓冲锁定
	CThreadLockHandle BufferLockHandle(&m_BufferLock);
	return m_pQueueService->GetUnCompleteCount();
}

//////////////////////////////////////////////////////////////////////////
