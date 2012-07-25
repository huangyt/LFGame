/*
* 文件        : AsynchronismEngine.h
* 版本        : 1.0
* 描述        : 异步引擎接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/21/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "DataStorage.h"

//////////////////////////////////////////////////////////////////////////

//异步引擎钩子接口
class CAsynchronismEngineSink
{
public:
	//启动事件
	virtual bool OnAsynchronismEngineStart()=NULL;
	//停止事件
	virtual bool OnAsynchronismEngineStop()=NULL;
	//异步请求
	virtual bool OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//类说明
class CAsynchronismEngine;
typedef class std::unordered_set<CAsynchronismEngineSink *> CAsynchronismEngineSinkSet;

//////////////////////////////////////////////////////////////////////////

//消息定义
struct stMsg
{
	WPARAM _wParam;
	LPARAM _lParam;
};
typedef std::queue<stMsg*>	MsgQueue;

//消息线程
class CMessageThread : public CServiceThread
{
	//函数定义
public:
	//构造函数
	CMessageThread();
	//析构函数
	virtual ~CMessageThread();

	//功能函数
public:
	void PostMessage(WPARAM wParam, LPARAM lParam);
	//停止线程
	virtual bool StopThread(DWORD dwWaitSeconds=INFINITE);

	//事件函数
private:
	//开始事件
	virtual bool OnThreadStratEvent();
	//停止事件
	virtual bool OnThreadStopEvent();

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();

	//消息映射
protected:
	//请求消息
	bool OnAsynRequest(WPARAM wParam, LPARAM lParam);

	//变量定义
public:
	CAsynchronismEngine				*m_pAsynchronismEngine;			//异步引擎
	MsgQueue						m_MsgQueue;						//消息队列
	HANDLE							m_hMsgEvent;					//消息事件
	bool							m_bRunning;						//消息循环标志
	CThreadLock						m_MsgLock;						//消息队列锁
};

//////////////////////////////////////////////////////////////////////////

//异步引擎
class CAsynchronismEngine
{
	friend class CMessageThread;

	//函数定义
public:
	//构造函数
	CAsynchronismEngine(void);
	//析构函数
	~CAsynchronismEngine(void);

	//管理接口
public:
	//启动服务
	bool BeginService();
	//停止服务
	bool EndService();
	//插入请求
	bool InsertRequest(WORD wRequestID, void * const pBuffer, WORD wDataSize, CAsynchronismEngineSink *pAsynchronismEngineSink);

	//功能接口
public:
	//注册钩子
	bool RegisterAsynchronismEngineSink(CAsynchronismEngineSink * pAsynchronismEngineSink);
	//取消注册
	bool UnRegisterAsynchronismEngineSink(CAsynchronismEngineSink * pAsynchronismEngineSink);

	//线程函数
protected:
	//线程启动
	bool OnMessageThreadStart();
	//线程停止
	bool OnMessageThreadStop();
	//异步请求
	void OnAsynchronismRequest(WORD wRequestID, CAsynchronismEngineSink * pAsynchronismEngineSink);

	//内核变量
protected:
	bool							m_bService;							//服务标志
	BYTE							m_cbBuffer[MAX_QUEUE_PACKET];		//数据缓冲
	CAsynchronismEngineSinkSet		m_AsynchronismEngineSinkSet;		//异步钩子

	//组件变量
protected:
	CThreadLock						m_ThreadLock;						//线程同步
	CDataStorage					m_DataStorage;						//数据存储
	CMessageThread					m_MessageThread;					//线程组件
};

//////////////////////////////////////////////////////////////////////////