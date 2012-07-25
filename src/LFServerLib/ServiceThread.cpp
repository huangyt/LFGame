#include "ServiceThread.h"
#include <Process.h>

//////////////////////////////////////////////////////////////////////////
//�ṹ����

//��������
struct tagThreadParameter
{
	bool							bSuccess;							//�Ƿ����
	HANDLE							hEventFinish;						//�¼����
	CServiceThread					*pServiceThread;					//�߳�ָ��
};

//////////////////////////////////////////////////////////////////////////

//���캯��
CThreadLockHandle::CThreadLockHandle(CThreadLock * pThreadLock, bool bAutoLock/* =true */)
{
	ASSERT(pThreadLock!=NULL);
	m_nLockCount=0;
	m_pThreadLock=pThreadLock;
	if (bAutoLock) Lock();
	return;
}

//��������
CThreadLockHandle::~CThreadLockHandle()
{
	while (m_nLockCount>0) UnLock();
}

//��������
void CThreadLockHandle::Lock()
{
	//Ч��״̬
	ASSERT(m_nLockCount>=0);
	ASSERT(m_pThreadLock!=NULL);

	//��������
	m_nLockCount++;
	m_pThreadLock->Lock();
}

//�������� 
void CThreadLockHandle::UnLock()
{
	//Ч��״̬
	ASSERT(m_nLockCount>0);
	ASSERT(m_pThreadLock!=NULL);

	//�������
	m_nLockCount--;
	m_pThreadLock->UnLock();
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CServiceThread::CServiceThread(void)
{
	m_bRun=false;
	m_uThreadID=0;
	m_hThreadHandle=NULL;
}

//��������
CServiceThread::~CServiceThread(void)
{
	StopThread(INFINITE);
}

//״̬�ж�
bool CServiceThread::IsRuning()
{
	if (m_hThreadHandle!=NULL)
	{
		DWORD dwRetCode=WaitForSingleObject(m_hThreadHandle,0);
		if (dwRetCode==WAIT_TIMEOUT) return true;
	}
	return false;
}

//�����߳�
bool CServiceThread::StartThead()
{
	//Ч��״̬
	if (IsRuning()) return false;

	//�������
	if (m_hThreadHandle!=NULL) 
	{
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle=NULL;
		m_uThreadID=0;
	}

	//��������
	tagThreadParameter ThreadParameter;
	ZeroMemory(&ThreadParameter,sizeof(ThreadParameter));

	//���ñ���
	ThreadParameter.bSuccess=false;
	ThreadParameter.pServiceThread=this;
	ThreadParameter.hEventFinish=CreateEvent(NULL,FALSE,FALSE,NULL);

	//Ч��״̬
	ASSERT(ThreadParameter.hEventFinish!=NULL);
	if (ThreadParameter.hEventFinish==NULL) return false;

	//�����߳�
	m_bRun=true;
	m_hThreadHandle=(HANDLE)::_beginthreadex(NULL,0,ThreadFunction,&ThreadParameter,0,&m_uThreadID);

	//�����ж�
	if (m_hThreadHandle==INVALID_HANDLE_VALUE)
	{
		CloseHandle(ThreadParameter.hEventFinish);
		return false;
	}

	//�ȴ��¼�
	WaitForSingleObject(ThreadParameter.hEventFinish,INFINITE);

	//�ر��¼�
	CloseHandle(ThreadParameter.hEventFinish);

	//�жϴ���
	if (ThreadParameter.bSuccess==false) 
	{
		StopThread();
		return false;
	}

	return true;
}

//ֹͣ�߳�
bool CServiceThread::StopThread(DWORD dwWaitSeconds)
{
	//ֹͣ�߳�
	if (IsRuning()==true)
	{
		m_bRun=false;
		DWORD dwRetCode=WaitForSingleObject(m_hThreadHandle,dwWaitSeconds);
		if (dwRetCode==WAIT_TIMEOUT) return false;
	}

	//���ñ���
	if (m_hThreadHandle!=NULL)
	{
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle=NULL;
		m_uThreadID=0;
	}

	return true;
}

//��ֹ�߳�
bool CServiceThread::TerminateThread(DWORD dwExitCode)
{
	//ֹͣ�߳�
	if (IsRuning()==true)
	{
		::TerminateThread(m_hThreadHandle,dwExitCode);
	}

	//���ñ���
	if (m_hThreadHandle!=NULL)
	{
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle=NULL;
		m_uThreadID=0;
	}

	return true;
}

//�̺߳���
unsigned __stdcall CServiceThread::ThreadFunction(LPVOID pThreadData)
{
	//��ȡ����
	ASSERT(pThreadData!=NULL);
	tagThreadParameter * pThreadParameter=(tagThreadParameter *)pThreadData;
	CServiceThread * pServiceThread=pThreadParameter->pServiceThread;

	//��������
	srand((DWORD)time(NULL));

	//�����¼�
	try 
	{
		pThreadParameter->bSuccess=pServiceThread->OnThreadStratEvent(); 
	} 
	catch (...) 
	{ 
		//���ñ���
		ASSERT(FALSE);
		pThreadParameter->bSuccess=false; 
	}

	//�����¼�
	bool bSuccess=pThreadParameter->bSuccess;
	ASSERT(pThreadParameter->hEventFinish!=NULL);
	if (pThreadParameter->hEventFinish!=NULL) SetEvent(pThreadParameter->hEventFinish);

	//�����߳�
	if (bSuccess==true)
	{
		//�߳�����
		while (pServiceThread->m_bRun)
		{
#ifndef _DEBUG
			//���а汾
			try
			{
				if (pServiceThread->RepetitionRun()==false)
				{
					break;
				}
			}
			catch (...)	{ }
#else
			//���԰汾
			if (pServiceThread->RepetitionRun()==false)
			{
				break;
			}
#endif
		}

		//ֹ֪ͣͨ
		try
		{ 
			pServiceThread->OnThreadStopEvent();
		} 
		catch (...)	{ ASSERT(FALSE); }
	}

	//��ֹ�߳�
	_endthreadex(0);

	return 0;
}

//////////////////////////////////////////////////////////////////////////