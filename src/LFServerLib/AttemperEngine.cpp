#include "AttemperEngine.h"

//////////////////////////////////////////////////////////////////////////

//构造函数
CAttemperEngine::CAttemperEngine(void)
{
	m_bService=false;
	m_pTCPSocketEngine=NULL;
	m_pAttemperEngineSink=NULL;
	return;
}

//析构函数
CAttemperEngine::~CAttemperEngine(void)
{
}

//启动服务
bool CAttemperEngine::BeginService()
{
	//判断状态
	if (m_bService==true) 
	{
		return true;
	}

	//外挂接口
	if (m_pAttemperEngineSink==NULL)
	{
		return false;
	}

	//设置队列
	m_RequestQueueService.SetQueueServiceSink(static_cast<CQueueServiceSink*>(this));

	//启动外挂
	if (m_pAttemperEngineSink->BeginService()==false)
	{
		return false;
	}

	//启动队列
	if (m_RequestQueueService.BeginService()==false)
	{
		return false;
	}

	//设置变量
	m_bService=true;

	return true;
}

//停止服务
bool CAttemperEngine::EndService()
{
	//设置变量
	m_bService=false;

	//停止请求队列
	m_RequestQueueService.EndService();

	//停止外挂
	if (m_pAttemperEngineSink!=NULL)
	{
		m_pAttemperEngineSink->EndService();
	}

	return true;
}

//设置网络
void CAttemperEngine::SetSocketEngine(CTCPSocketEngine *pTCPSocketEngine)
{
	ASSERT(pTCPSocketEngine!=NULL);
	m_pTCPSocketEngine = pTCPSocketEngine;
}

//注册钩子
void CAttemperEngine::SetAttemperEngineSink(CAttemperEngineSink *pAttemperEngineSink)
{
	//效验参数
	ASSERT(pAttemperEngineSink!=NULL);

	//查询接口
	m_pAttemperEngineSink  = pAttemperEngineSink;
}

//获取接口
CQueueService* CAttemperEngine::GetQueueService()
{
	return &m_RequestQueueService;
}

//队列接口
void CAttemperEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	//内核事件
	ASSERT(m_pAttemperEngineSink!=NULL);
	switch (wIdentifier)
	{
	case EVENT_TIMER:			//定时器事件
		{
			//效验参数
			ASSERT(wDataSize==sizeof(NTY_TimerEvent));
			if (wDataSize!=sizeof(NTY_TimerEvent)) return;

			//处理消息
			NTY_TimerEvent * pTimerEvent=(NTY_TimerEvent *)pBuffer;
			try
			{
				m_pAttemperEngineSink->OnEventTimer(pTimerEvent->wTimerID,pTimerEvent->wBindParam);
			}
			catch (...){ }

			return;
		}
	case EVENT_DATABASE:		//数据库事件
		{
			//效验参数
			ASSERT(wDataSize>=sizeof(NTY_DataBaseEvent));
			if (wDataSize<sizeof(NTY_DataBaseEvent)) return;

			//处理消息
			NTY_DataBaseEvent * pDataBaseEvent=(NTY_DataBaseEvent *)pBuffer;
			if(pDataBaseEvent->wDataSize == 0)
			{
				pDataBaseEvent->pDataBuffer = NULL;
			}
			else
			{
				pDataBaseEvent->pDataBuffer=pDataBaseEvent+1;
			}
			try
			{
				m_pAttemperEngineSink->OnEventDataBase(pDataBaseEvent);
			}
			catch (...){ }

			return;
		}
	case EVENT_SOCKET_ACCEPT:	//网络应答事件
		{
			//效验大小
			ASSERT(wDataSize==sizeof(NTY_SocketAcceptEvent));
			if (wDataSize!=sizeof(NTY_SocketAcceptEvent)) return;

			//处理消息
			NTY_SocketAcceptEvent * pSocketAcceptEvent=(NTY_SocketAcceptEvent *)pBuffer;
			bool bSuccess=false;
			try 
			{ 
				bSuccess = m_pAttemperEngineSink->OnEventSocketAccept(pSocketAcceptEvent);
			}
			catch (...)	{ }
			if (bSuccess==false) m_pTCPSocketEngine->CloseSocket(pSocketAcceptEvent->dwSocketID);


			return;
		}
	case EVENT_SOCKET_READ:		//网络读取事件
		{
			//效验大小
			NTY_SocketReadEvent * pSocketReadEvent=(NTY_SocketReadEvent *)pBuffer;
			ASSERT(wDataSize>=sizeof(NTY_SocketReadEvent));
			ASSERT(wDataSize==(sizeof(NTY_SocketReadEvent)+pSocketReadEvent->wDataSize));
			if (wDataSize<sizeof(NTY_SocketReadEvent)) return;
			if (wDataSize!=(sizeof(NTY_SocketReadEvent)+pSocketReadEvent->wDataSize)) return;

			//处理消息
			bool bSuccess=false;
			try 
			{ 
				if(pSocketReadEvent->wDataSize == 0)
				{
					pSocketReadEvent->pDataBuffer = NULL;
				}
				else
				{
					pSocketReadEvent->pDataBuffer=pSocketReadEvent+1;
				}
				bSuccess=m_pAttemperEngineSink->OnEventSocketRead(pSocketReadEvent);
			}
			catch (...)	{ }
			if (bSuccess==false) m_pTCPSocketEngine->CloseSocket(pSocketReadEvent->dwSocketID);

			return;
		}
	case EVENT_SOCKET_CLOSE:	//网络关闭事件
		{
			//效验大小
			ASSERT(wDataSize==sizeof(NTY_SocketCloseEvent));
			if (wDataSize!=sizeof(NTY_SocketCloseEvent)) return;

			//处理消息
			NTY_SocketCloseEvent * pSocketCloseEvent=(NTY_SocketCloseEvent *)pBuffer;
			bool bSuccess = false;
			try
			{
				bSuccess = m_pAttemperEngineSink->OnEventSocketClose(pSocketCloseEvent);
			}
			catch (...)	{ }
			if (bSuccess==false) m_pTCPSocketEngine->CloseSocket(pSocketCloseEvent->dwSocketID);

			return;
		}
	}

	//其他事件
	m_pAttemperEngineSink->OnAttemperEvent(wIdentifier, pBuffer, wDataSize); 

	return;
}