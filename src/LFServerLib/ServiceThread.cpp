#include "ServiceThread.h"
#include <Process.h>

//////////////////////////////////////////////////////////////////////////
//结构定义

//启动参数
struct tagThreadParameter
{
	bool							bSuccess;							//是否错误
	HANDLE							hEventFinish;						//事件句柄
	CServiceThread					*pServiceThread;					//线程指针
};

//////////////////////////////////////////////////////////////////////////

//构造函数
CThreadLockHandle::CThreadLockHandle(CThreadLock * pThreadLock, bool bAutoLock/* =true */)
{
	ASSERT(pThreadLock!=NULL);
	m_nLockCount=0;
	m_pThreadLock=pThreadLock;
	if (bAutoLock) Lock();
	return;
}

//析构函数
CThreadLockHandle::~CThreadLockHandle()
{
	while (m_nLockCount>0) UnLock();
}

//锁定函数
void CThreadLockHandle::Lock()
{
	//效验状态
	ASSERT(m_nLockCount>=0);
	ASSERT(m_pThreadLock!=NULL);

	//锁定对象
	m_nLockCount++;
	m_pThreadLock->Lock();
}

//解锁函数 
void CThreadLockHandle::UnLock()
{
	//效验状态
	ASSERT(m_nLockCount>0);
	ASSERT(m_pThreadLock!=NULL);

	//解除锁定
	m_nLockCount--;
	m_pThreadLock->UnLock();
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CServiceThread::CServiceThread(void)
{
	m_bRun=false;
	m_uThreadID=0;
	m_hThreadHandle=NULL;
}

//析构函数
CServiceThread::~CServiceThread(void)
{
	StopThread(INFINITE);
}

//状态判断
bool CServiceThread::IsRuning()
{
	if (m_hThreadHandle!=NULL)
	{
		DWORD dwRetCode=WaitForSingleObject(m_hThreadHandle,0);
		if (dwRetCode==WAIT_TIMEOUT) return true;
	}
	return false;
}

//启动线程
bool CServiceThread::StartThead()
{
	//效验状态
	if (IsRuning()) return false;

	//清理变量
	if (m_hThreadHandle!=NULL) 
	{
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle=NULL;
		m_uThreadID=0;
	}

	//变量定义
	tagThreadParameter ThreadParameter;
	ZeroMemory(&ThreadParameter,sizeof(ThreadParameter));

	//设置变量
	ThreadParameter.bSuccess=false;
	ThreadParameter.pServiceThread=this;
	ThreadParameter.hEventFinish=CreateEvent(NULL,FALSE,FALSE,NULL);

	//效验状态
	ASSERT(ThreadParameter.hEventFinish!=NULL);
	if (ThreadParameter.hEventFinish==NULL) return false;

	//启动线程
	m_bRun=true;
	m_hThreadHandle=(HANDLE)::_beginthreadex(NULL,0,ThreadFunction,&ThreadParameter,0,&m_uThreadID);

	//错误判断
	if (m_hThreadHandle==INVALID_HANDLE_VALUE)
	{
		CloseHandle(ThreadParameter.hEventFinish);
		return false;
	}

	//等待事件
	WaitForSingleObject(ThreadParameter.hEventFinish,INFINITE);

	//关闭事件
	CloseHandle(ThreadParameter.hEventFinish);

	//判断错误
	if (ThreadParameter.bSuccess==false) 
	{
		StopThread();
		return false;
	}

	return true;
}

//停止线程
bool CServiceThread::StopThread(DWORD dwWaitSeconds)
{
	//停止线程
	if (IsRuning()==true)
	{
		m_bRun=false;
		DWORD dwRetCode=WaitForSingleObject(m_hThreadHandle,dwWaitSeconds);
		if (dwRetCode==WAIT_TIMEOUT) return false;
	}

	//设置变量
	if (m_hThreadHandle!=NULL)
	{
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle=NULL;
		m_uThreadID=0;
	}

	return true;
}

//中止线程
bool CServiceThread::TerminateThread(DWORD dwExitCode)
{
	//停止线程
	if (IsRuning()==true)
	{
		::TerminateThread(m_hThreadHandle,dwExitCode);
	}

	//设置变量
	if (m_hThreadHandle!=NULL)
	{
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle=NULL;
		m_uThreadID=0;
	}

	return true;
}

//线程函数
unsigned __stdcall CServiceThread::ThreadFunction(LPVOID pThreadData)
{
	//获取参数
	ASSERT(pThreadData!=NULL);
	tagThreadParameter * pThreadParameter=(tagThreadParameter *)pThreadData;
	CServiceThread * pServiceThread=pThreadParameter->pServiceThread;

	//设置因子
	srand((DWORD)time(NULL));

	//启动事件
	try 
	{
		pThreadParameter->bSuccess=pServiceThread->OnThreadStratEvent(); 
	} 
	catch (...) 
	{ 
		//设置变量
		ASSERT(FALSE);
		pThreadParameter->bSuccess=false; 
	}

	//设置事件
	bool bSuccess=pThreadParameter->bSuccess;
	ASSERT(pThreadParameter->hEventFinish!=NULL);
	if (pThreadParameter->hEventFinish!=NULL) SetEvent(pThreadParameter->hEventFinish);

	//运行线程
	if (bSuccess==true)
	{
		//线程运行
		while (pServiceThread->m_bRun)
		{
#ifndef _DEBUG
			//运行版本
			try
			{
				if (pServiceThread->RepetitionRun()==false)
				{
					break;
				}
			}
			catch (...)	{ }
#else
			//调试版本
			if (pServiceThread->RepetitionRun()==false)
			{
				break;
			}
#endif
		}

		//停止通知
		try
		{ 
			pServiceThread->OnThreadStopEvent();
		} 
		catch (...)	{ ASSERT(FALSE); }
	}

	//中止线程
	_endthreadex(0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////