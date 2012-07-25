#include "TimerEngine.h"
#include "LogEngine.h"
#include "SystemTime.h"

//宏定义
#define NO_TIME_LEFT						DWORD(-1)				//不响应时间

//////////////////////////////////////////////////////////////////////////

//构造函数
CTimerThread::CTimerThread(void)
{
	m_dwTimerSpace=0L;
	m_pTimerEngine=NULL;
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//析构函数
CTimerThread::~CTimerThread(void)
{
	CloseHandle(m_hEvent);
}

//配置函数
bool CTimerThread::InitThread(CTimerEngine * pTimerEngine, DWORD dwTimerSpace)
{
	//效益参数
	ASSERT(dwTimerSpace>=1L);
	ASSERT(pTimerEngine!=NULL);
	if (pTimerEngine==NULL) return false;

	//设置变量
	m_dwTimerSpace=dwTimerSpace;
	m_pTimerEngine=pTimerEngine;

	return true;
}

//运行函数
bool CTimerThread::RepetitionRun()
{
	ASSERT(m_pTimerEngine!=NULL);
	WaitForSingleObject(m_hEvent, m_dwTimerSpace);
	m_pTimerEngine->OnTimerThreadSink();
	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CTimerEngine::CTimerEngine(void)
{
	m_bService=false;
	m_dwTimePass=0L;
	m_dwTimerSpace=30L;
	m_dwTimeLeave=NO_TIME_LEFT;
}

//析构函数
CTimerEngine::~CTimerEngine(void)
{
	INT_PTR i = 0;
	//停止服务
	EndService();

	//清理内存
	for(CTimerItemMap::iterator iter = m_TimerItemFree.begin();
		iter != m_TimerItemFree.end(); ++iter)
	{
		delete iter->second;
	}

	for(CTimerItemMap::iterator iter = m_TimerItemActive.begin();
		iter != m_TimerItemActive.end(); ++iter)
	{
		delete iter->second;
	}
	m_TimerItemFree.clear();
	m_TimerItemActive.clear();
}

//设置定时器
bool CTimerEngine::SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam)
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//效验参数
	ASSERT(dwRepeat>0L);
	if (dwRepeat==0) return false;
	dwElapse=(dwElapse+m_dwTimerSpace-1)/m_dwTimerSpace*m_dwTimerSpace;

	//查找定时器
	bool bTimerExist=false;
	tagTimerItem * pTimerItem=NULL;
	CTimerItemMap::iterator iter = m_TimerItemActive.find(wTimerID);
	if(iter != m_TimerItemActive.end())
	{
		pTimerItem = iter->second;
		ASSERT(pTimerItem!=NULL);
		if (pTimerItem->wTimerID==wTimerID) 
		{
			bTimerExist=true;
		}
	}

	//创建定时器
	if (bTimerExist==false)
	{
		UINT nFreeCount=m_TimerItemFree.size();
		if (nFreeCount>0)
		{
			pTimerItem=m_TimerItemFree.begin()->second;
			ASSERT(pTimerItem!=NULL);
			m_TimerItemFree.erase(m_TimerItemFree.begin());
		}
		else
		{
			try
			{
				pTimerItem=new tagTimerItem;
				ASSERT(pTimerItem!=NULL);
				if (pTimerItem==NULL) return false;
			}
			catch (...) { return false; }
		}
	}

	//设置参数
	ASSERT(pTimerItem!=NULL);
	pTimerItem->wTimerID=wTimerID;
	pTimerItem->wBindParam=wParam;
	pTimerItem->dwElapse=dwElapse;
	pTimerItem->dwRepeatTimes=dwRepeat;
	pTimerItem->dwTimeLeave=dwElapse+m_dwTimePass;

	//激活定时器
	m_dwTimeLeave=__min(m_dwTimeLeave,dwElapse);
	if (bTimerExist==false) 
	{
		m_TimerItemActive.insert(std::make_pair(wTimerID, pTimerItem));
		return true;
	}

	return false;
}

//删除定时器
bool CTimerEngine::KillTimer(WORD wTimerID)
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//查找定时器
	CTimerItemMap::iterator iter = m_TimerItemActive.find(wTimerID);
	if(iter != m_TimerItemActive.end())
	{
		m_TimerItemFree.insert(std::make_pair(wTimerID,iter->second));
		m_TimerItemActive.erase(iter);
		if(m_TimerItemActive.empty())
		{
			m_dwTimePass=0L;
			m_dwTimeLeave=NO_TIME_LEFT;
		}
	}

	return false;
}

//删除定时器
bool CTimerEngine::KillAllTimer()
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//删除定时器
	m_TimerItemFree.insert(m_TimerItemActive.begin(), m_TimerItemActive.end());
	m_TimerItemActive.clear();

	//设置变量
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;

	return true;
}

//开始服务
bool CTimerEngine::BeginService()
{
	//效验状态
	if (m_bService==true) 
	{
		return true;
	}

	//设置变量
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;
	if (m_TimerThread.InitThread(this,m_dwTimerSpace)==false)
	{
		return false;
	}

	//启动服务
	if (m_TimerThread.StartThead()==false)
	{
		return false;
	}

	SetThreadPriority(m_TimerThread.GetThreadHandle(), REALTIME_PRIORITY_CLASS);

	//设置变量
	m_bService=true;

	return true;
}

//停止服务
bool CTimerEngine::EndService()
{
	//效验状态
	if (m_bService==false) 
	{
		return true;
	}

	//设置变量
	m_bService=false;

	//停止线程
	m_TimerThread.StopThread();

	//设置变量
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;

	m_TimerItemFree.insert(m_TimerItemActive.begin(), m_TimerItemActive.end());
	m_TimerItemActive.clear();

	return true;
}

//设置接口
void CTimerEngine::SetTimerEngineSink(CQueueServiceBase *pQueueServiceBase)
{
	ASSERT(pQueueServiceBase!=NULL);
	//效验参数
	m_AttemperEvent.SetQueueService(pQueueServiceBase);
}

//定时器通知
void CTimerEngine::OnTimerThreadSink()
{
	//锁定资源
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//倒计时间
	if (m_dwTimeLeave==NO_TIME_LEFT) 
	{
		ASSERT(m_TimerItemActive.size()==0);
		return;
	}

	//减少时间
	ASSERT(m_dwTimeLeave>=m_dwTimerSpace);
	m_dwTimeLeave-=m_dwTimerSpace;
	m_dwTimePass+=m_dwTimerSpace;

	//查询定时器
	if (m_dwTimeLeave==0)
	{
		bool bKillTimer=false;
		tagTimerItem * pTimerItem=NULL;
		DWORD dwTimeLeave=NO_TIME_LEFT;
		for (CTimerItemMap::iterator iter = m_TimerItemActive.begin();iter != m_TimerItemActive.end();)
		{
			//效验参数
			pTimerItem=iter->second;
			ASSERT(pTimerItem!=NULL);
			ASSERT(pTimerItem->dwTimeLeave>=m_dwTimePass);

			//定时器处理
			bKillTimer=false;
			pTimerItem->dwTimeLeave-=m_dwTimePass;

			if (pTimerItem->dwTimeLeave==0L)
			{
				//发送通知
				m_AttemperEvent.PostTimerEvent(pTimerItem->wTimerID,pTimerItem->wBindParam);

				//设置次数
				if (pTimerItem->dwRepeatTimes!=INFINITE)
				{
					ASSERT(pTimerItem->dwRepeatTimes>0);
					if (pTimerItem->dwRepeatTimes==1L)
					{
						bKillTimer=true;
						iter = m_TimerItemActive.erase(iter);
						m_TimerItemFree.insert(std::make_pair(pTimerItem->wTimerID,pTimerItem));
					}
					else pTimerItem->dwRepeatTimes--;
				}

				//设置时间
				if (bKillTimer==false) pTimerItem->dwTimeLeave=pTimerItem->dwElapse;
			}

			//增加索引
			if (bKillTimer==false) 
			{
				++iter;
				dwTimeLeave=__min(dwTimeLeave,pTimerItem->dwTimeLeave);
				ASSERT(dwTimeLeave%m_dwTimerSpace==0);
			}
		}

		//设置响应
		m_dwTimePass=0L;
		m_dwTimeLeave=dwTimeLeave;
	}
}


//////////////////////////////////////////////////////////////////////////