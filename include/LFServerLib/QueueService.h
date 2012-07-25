/*
* 文件        : QueueService.h
* 版本        : 1.0
* 描述        : 队列服务接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        李峰           创建
* 
*/
#pragma once

//头文件引用
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "DataStorage.h"

//////////////////////////////////////////////////////////////////////////

//数据队列类钩子接口
class CQueueServiceSink
{
public:
	//通知回调函数
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)=NULL;
};

//数据队列接口
class CQueueServiceBase
{
public:
	virtual LONG GetUnCompleteCount()=NULL;
	//加入数据
	virtual bool AddToQueue(WORD wIdentifier, void * const pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//队列线程类
class CQueueServiceThread : public CServiceThread
{
	//友员定义
	friend class CQueueService;

	//配置定义
protected:
	HANDLE							m_hCompletionPort;					//完成端口

	//辅助变量
private:
	BYTE							m_cbBuffer[MAX_QUEUE_PACKET];		//接收缓冲

	//函数定义
protected:
	//构造函数
	CQueueServiceThread(void);
	//析构函数
	virtual ~CQueueServiceThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(HANDLE hCompletionPort);
	//取消配置
	bool UnInitThread();

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//数据队列类
class CQueueService : public CQueueServiceBase
{
	friend class CQueueServiceThread;

	//函数定义
public:
	//构造函数
	CQueueService(void);
	//析构函数
	virtual ~CQueueService(void);

	//队列接口
public:
	//加入数据
	virtual bool AddToQueue(WORD wIdentifier, void * const pBuffer, WORD wDataSize);
	//得到未完成任务数目
	virtual LONG GetUnCompleteCount();

	//管理接口
public:
	//开始服务
	bool BeginService();
	//停止服务
	bool EndService();
	//设置接口
	void SetQueueServiceSink(CQueueServiceSink *pQueueServiceSink);
	//负荷信息
	bool GetBurthenInfo(tagBurthenInfo & BurthenInfo);

	//辅助函数
private:
	//提取数据
	bool GetData(tagDataHead & DataHead, void * pBuffer, WORD wBufferSize);
	//数据消息
	void OnQueueServiceThread(const tagDataHead & DataHead, void * pBuffer, WORD wDataSize);

	//变量定义
protected:
	bool							m_bService;							//服务标志
	HANDLE							m_hCompletionPort;					//完成端口
	CQueueServiceSink				*m_pQueueServiceSink;				//回调接口
	LONG							m_lUnComplete;
	//组件变量
protected:
	CThreadLock						m_ThreadLock;						//线程锁
	CDataStorage					m_DataStorage;						//数据存储
	CQueueServiceThread				m_QueueServiceThread;				//队列线程
};

//////////////////////////////////////////////////////////////////////////

//数据队列事件
class CQueueServiceEvent
{
	//函数定义
public:
	//构造函数
	CQueueServiceEvent();
	//析构函数
	virtual ~CQueueServiceEvent();

	//管理函数
public:
	//设置接口
	void SetQueueService(CQueueServiceBase *pQueueService);

	//通知函数
public:
	//定时器事件
	bool PostTimerEvent(WORD wTimerID, WPARAM wBindParam);
	//数据库事件
	bool PostDataBaseEvent(WORD wRequestID, DWORD dwSocketID, const void * pDataBuffer, WORD wDataSize);
	//网络应答事件
	bool PostSocketAcceptEvent(DWORD dwSocketID, DWORD dwClientIP);
	//网络读取事件
	bool PostSocketReadEvent(DWORD dwSocketID, DWORD dwCommand, const void * pDataBuffer, WORD wDataSize);
	//网络关闭事件
	bool PostSocketCloseEvent(DWORD dwSocketID, DWORD dwClientIP, DWORD dwConnectSecond);

	LONG GetUnCompleteCount();

	//变量定义
public:
	CThreadLock						m_BufferLock;						//同步锁定
	CQueueServiceBase				*m_pQueueService;					//队列接口
	BYTE							m_cbBuffer[MAX_QUEUE_PACKET];		//接收缓冲
};

//////////////////////////////////////////////////////////////////////////
