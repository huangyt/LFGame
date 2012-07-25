/*
* �ļ�        : ServiceThread.h
* �汾        : 1.0
* ����        : �����߳̽ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        ���           ����
* 
*/
#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"

//////////////////////////////////////////////////////////////////////////
//�ӿڶ���
#define CLASS_UNCOPYABLE(classname) private: classname##(const classname##&); classname##& operator=(const classname##&);

//�ٽ���ͬ����
class CThreadLock
{
	CLASS_UNCOPYABLE(CThreadLock)
	//��������
public:
	//���캯��
	inline CThreadLock() { ::InitializeCriticalSection(&m_csLock); }

	//��������
	inline ~CThreadLock() { ::DeleteCriticalSection(&m_csLock); }

	//���ܺ���
public:
	//��������
	virtual inline void Lock() { ::EnterCriticalSection(&m_csLock); }
	//�������� 
	virtual inline void UnLock() { ::LeaveCriticalSection(&m_csLock); }

	//��������
private:
	CRITICAL_SECTION					m_csLock;					//�ٽ����
};

//////////////////////////////////////////////////////////////////////////

//��ȫͬ���������
class CThreadLockHandle
{
	CLASS_UNCOPYABLE(CThreadLockHandle)
	//��������
public:
	//���캯��
	CThreadLockHandle(CThreadLock * pThreadLock, bool bAutoLock=true);
	//��������
	virtual ~CThreadLockHandle();

	//���ܺ���
public:
	//��������
	void Lock();
	//�������� 
	void UnLock();
	//��ȡ��������
	inline int GetLockCount() { return m_nLockCount; }

	//��������
private:
	int								m_nLockCount;				//��������
	CThreadLock						*m_pThreadLock;				//��������
};

//////////////////////////////////////////////////////////////////////////

//�̶߳�����
class CServiceThread
{
	//��������
protected:
	//���캯��
	CServiceThread(void);
	//��������
	virtual ~CServiceThread(void);

	//�ӿں���
public:
	//��ȡ״̬
	virtual bool IsRuning();
	//�����߳�
	virtual bool StartThead();
	//ֹͣ�߳�
	virtual bool StopThread(DWORD dwWaitSeconds=INFINITE);
	//��ֹ�߳�
	virtual bool TerminateThread(DWORD dwExitCode);

	//���ܺ���
public:
	//��ȡ��ʶ
	UINT GetThreadID() { return m_uThreadID; }
	//��ȡ���
	HANDLE GetThreadHandle() { return m_hThreadHandle; }

	//�¼�����
private:
	//��ʼ�¼�
	virtual bool OnThreadStratEvent() { return true; }
	//ֹͣ�¼�
	virtual bool OnThreadStopEvent() { return true; }

	//�ڲ�����
private:
	//���к���
	virtual bool RepetitionRun()=NULL;
	//�̺߳���
	static unsigned __stdcall ThreadFunction(LPVOID pThreadData);

	//��������
private:
	volatile bool						m_bRun;							//���б�־
	UINT								m_uThreadID;					//�̱߳�ʶ
	HANDLE								m_hThreadHandle;				//�߳̾��
};