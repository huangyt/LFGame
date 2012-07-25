/*
* 文件        : AttemperEngine.h
* 版本        : 1.0
* 描述        : 调度引擎接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/20/2010  1.0        李峰           创建
* 
*/
#pragma once

//头文件引用
#include "GlobalDef.h"
#include "SocketEngine.h"

//////////////////////////////////////////////////////////////////////////

//调度模块钩子接口
class CAttemperEngineSink
{
	//管理接口
public:
	//调度模块启动
	virtual bool BeginService()=NULL;
	//调度模块关闭
	virtual bool EndService()=NULL;
	//事件处理接口
	virtual bool OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize)=NULL;

	//事件接口
public:
	//定时器事件
	virtual bool OnEventTimer(WORD wTimerID, WPARAM wBindParam)=NULL;
	//数据库事件
	virtual bool OnEventDataBase(NTY_DataBaseEvent * pDataBaseEvent)=NULL;
	//网络应答事件
	virtual bool OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent)=NULL;
	//网络读取事件
	virtual bool OnEventSocketRead(NTY_SocketReadEvent * pSocketReadEvent)=NULL;
	//网络关闭事件
	virtual bool OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//调度管理类
class CAttemperEngine : public CQueueServiceSink
{
	//函数定义
public:
	//构造函数
	CAttemperEngine(void);
	//析构函数
	~CAttemperEngine(void);

	//管理接口
public:
	//启动服务
	bool BeginService();
	//停止服务
	bool EndService();
	//设置网络
	void SetSocketEngine(CTCPSocketEngine *pTCPSocketEngine);
	//注册钩子
	void SetAttemperEngineSink(CAttemperEngineSink *pAttemperEngineSink);
	//获取接口
	CQueueService* GetQueueService();

	//队列接口
public:
	//触发接口
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//核心变量
protected:
	bool							m_bService;							//运行标志
	CQueueService					m_RequestQueueService;				//队列对象

	//接口变量
protected:
	CTCPSocketEngine				* m_pTCPSocketEngine;				//网络引擎
	CAttemperEngineSink				* m_pAttemperEngineSink;			//挂接接口
};
