/*
* 文件        : ServiceThread.h
* 版本        : 1.0
* 描述        : 服务线程接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        李峰           创建
* 
*/
#pragma once

//头文件引用
#include "GlobalDef.h"

//////////////////////////////////////////////////////////////////////////
//接口定义
#define CLASS_UNCOPYABLE(classname) private: classname##(const classname##&); classname##& operator=(const classname##&);

//临界区同步类
class CThreadLock
{
	CLASS_UNCOPYABLE(CThreadLock)
	//函数定义
public:
	//构造函数
	inline CThreadLock() { ::InitializeCriticalSection(&m_csLock); }

	//析构函数
	inline ~CThreadLock() { ::DeleteCriticalSection(&m_csLock); }

	//功能函数
public:
	//锁定函数
	virtual inline void Lock() { ::EnterCriticalSection(&m_csLock); }
	//解锁函数 
	virtual inline void UnLock() { ::LeaveCriticalSection(&m_csLock); }

	//变量定义
private:
	CRITICAL_SECTION					m_csLock;					//临界变量
};

//////////////////////////////////////////////////////////////////////////

//安全同步锁定句柄
class CThreadLockHandle
{
	CLASS_UNCOPYABLE(CThreadLockHandle)
	//函数定义
public:
	//构造函数
	CThreadLockHandle(CThreadLock * pThreadLock, bool bAutoLock=true);
	//析构函数
	virtual ~CThreadLockHandle();

	//功能函数
public:
	//锁定函数
	void Lock();
	//解锁函数 
	void UnLock();
	//获取锁定次数
	inline int GetLockCount() { return m_nLockCount; }

	//变量定义
private:
	int								m_nLockCount;				//锁定计数
	CThreadLock						*m_pThreadLock;				//锁定对象
};

//////////////////////////////////////////////////////////////////////////

//线程对象类
class CServiceThread
{
	//函数定义
protected:
	//构造函数
	CServiceThread(void);
	//析构函数
	virtual ~CServiceThread(void);

	//接口函数
public:
	//获取状态
	virtual bool IsRuning();
	//启动线程
	virtual bool StartThead();
	//停止线程
	virtual bool StopThread(DWORD dwWaitSeconds=INFINITE);
	//中止线程
	virtual bool TerminateThread(DWORD dwExitCode);

	//功能函数
public:
	//获取标识
	UINT GetThreadID() { return m_uThreadID; }
	//获取句柄
	HANDLE GetThreadHandle() { return m_hThreadHandle; }

	//事件函数
private:
	//开始事件
	virtual bool OnThreadStratEvent() { return true; }
	//停止事件
	virtual bool OnThreadStopEvent() { return true; }

	//内部函数
private:
	//运行函数
	virtual bool RepetitionRun()=NULL;
	//线程函数
	static unsigned __stdcall ThreadFunction(LPVOID pThreadData);

	//变量定义
private:
	volatile bool						m_bRun;							//运行标志
	UINT								m_uThreadID;					//线程标识
	HANDLE								m_hThreadHandle;				//线程句柄
};