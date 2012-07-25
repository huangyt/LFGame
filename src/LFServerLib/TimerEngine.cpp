#include "TimerEngine.h"
#include "LogEngine.h"
#include "SystemTime.h"

//�궨��
#define NO_TIME_LEFT						DWORD(-1)				//����Ӧʱ��

//////////////////////////////////////////////////////////////////////////

//���캯��
CTimerThread::CTimerThread(void)
{
	m_dwTimerSpace=0L;
	m_pTimerEngine=NULL;
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//��������
CTimerThread::~CTimerThread(void)
{
	CloseHandle(m_hEvent);
}

//���ú���
bool CTimerThread::InitThread(CTimerEngine * pTimerEngine, DWORD dwTimerSpace)
{
	//Ч�����
	ASSERT(dwTimerSpace>=1L);
	ASSERT(pTimerEngine!=NULL);
	if (pTimerEngine==NULL) return false;

	//���ñ���
	m_dwTimerSpace=dwTimerSpace;
	m_pTimerEngine=pTimerEngine;

	return true;
}

//���к���
bool CTimerThread::RepetitionRun()
{
	ASSERT(m_pTimerEngine!=NULL);
	WaitForSingleObject(m_hEvent, m_dwTimerSpace);
	m_pTimerEngine->OnTimerThreadSink();
	return true;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CTimerEngine::CTimerEngine(void)
{
	m_bService=false;
	m_dwTimePass=0L;
	m_dwTimerSpace=30L;
	m_dwTimeLeave=NO_TIME_LEFT;
}

//��������
CTimerEngine::~CTimerEngine(void)
{
	INT_PTR i = 0;
	//ֹͣ����
	EndService();

	//�����ڴ�
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

//���ö�ʱ��
bool CTimerEngine::SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam)
{
	//������Դ
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//Ч�����
	ASSERT(dwRepeat>0L);
	if (dwRepeat==0) return false;
	dwElapse=(dwElapse+m_dwTimerSpace-1)/m_dwTimerSpace*m_dwTimerSpace;

	//���Ҷ�ʱ��
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

	//������ʱ��
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

	//���ò���
	ASSERT(pTimerItem!=NULL);
	pTimerItem->wTimerID=wTimerID;
	pTimerItem->wBindParam=wParam;
	pTimerItem->dwElapse=dwElapse;
	pTimerItem->dwRepeatTimes=dwRepeat;
	pTimerItem->dwTimeLeave=dwElapse+m_dwTimePass;

	//���ʱ��
	m_dwTimeLeave=__min(m_dwTimeLeave,dwElapse);
	if (bTimerExist==false) 
	{
		m_TimerItemActive.insert(std::make_pair(wTimerID, pTimerItem));
		return true;
	}

	return false;
}

//ɾ����ʱ��
bool CTimerEngine::KillTimer(WORD wTimerID)
{
	//������Դ
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//���Ҷ�ʱ��
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

//ɾ����ʱ��
bool CTimerEngine::KillAllTimer()
{
	//������Դ
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//ɾ����ʱ��
	m_TimerItemFree.insert(m_TimerItemActive.begin(), m_TimerItemActive.end());
	m_TimerItemActive.clear();

	//���ñ���
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;

	return true;
}

//��ʼ����
bool CTimerEngine::BeginService()
{
	//Ч��״̬
	if (m_bService==true) 
	{
		return true;
	}

	//���ñ���
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;
	if (m_TimerThread.InitThread(this,m_dwTimerSpace)==false)
	{
		return false;
	}

	//��������
	if (m_TimerThread.StartThead()==false)
	{
		return false;
	}

	SetThreadPriority(m_TimerThread.GetThreadHandle(), REALTIME_PRIORITY_CLASS);

	//���ñ���
	m_bService=true;

	return true;
}

//ֹͣ����
bool CTimerEngine::EndService()
{
	//Ч��״̬
	if (m_bService==false) 
	{
		return true;
	}

	//���ñ���
	m_bService=false;

	//ֹͣ�߳�
	m_TimerThread.StopThread();

	//���ñ���
	m_dwTimePass=0L;
	m_dwTimeLeave=NO_TIME_LEFT;

	m_TimerItemFree.insert(m_TimerItemActive.begin(), m_TimerItemActive.end());
	m_TimerItemActive.clear();

	return true;
}

//���ýӿ�
void CTimerEngine::SetTimerEngineSink(CQueueServiceBase *pQueueServiceBase)
{
	ASSERT(pQueueServiceBase!=NULL);
	//Ч�����
	m_AttemperEvent.SetQueueService(pQueueServiceBase);
}

//��ʱ��֪ͨ
void CTimerEngine::OnTimerThreadSink()
{
	//������Դ
	CThreadLockHandle LockHandle(&m_ThreadLock);

	//����ʱ��
	if (m_dwTimeLeave==NO_TIME_LEFT) 
	{
		ASSERT(m_TimerItemActive.size()==0);
		return;
	}

	//����ʱ��
	ASSERT(m_dwTimeLeave>=m_dwTimerSpace);
	m_dwTimeLeave-=m_dwTimerSpace;
	m_dwTimePass+=m_dwTimerSpace;

	//��ѯ��ʱ��
	if (m_dwTimeLeave==0)
	{
		bool bKillTimer=false;
		tagTimerItem * pTimerItem=NULL;
		DWORD dwTimeLeave=NO_TIME_LEFT;
		for (CTimerItemMap::iterator iter = m_TimerItemActive.begin();iter != m_TimerItemActive.end();)
		{
			//Ч�����
			pTimerItem=iter->second;
			ASSERT(pTimerItem!=NULL);
			ASSERT(pTimerItem->dwTimeLeave>=m_dwTimePass);

			//��ʱ������
			bKillTimer=false;
			pTimerItem->dwTimeLeave-=m_dwTimePass;

			if (pTimerItem->dwTimeLeave==0L)
			{
				//����֪ͨ
				m_AttemperEvent.PostTimerEvent(pTimerItem->wTimerID,pTimerItem->wBindParam);

				//���ô���
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

				//����ʱ��
				if (bKillTimer==false) pTimerItem->dwTimeLeave=pTimerItem->dwElapse;
			}

			//��������
			if (bKillTimer==false) 
			{
				++iter;
				dwTimeLeave=__min(dwTimeLeave,pTimerItem->dwTimeLeave);
				ASSERT(dwTimeLeave%m_dwTimerSpace==0);
			}
		}

		//������Ӧ
		m_dwTimePass=0L;
		m_dwTimeLeave=dwTimeLeave;
	}
}


//////////////////////////////////////////////////////////////////////////