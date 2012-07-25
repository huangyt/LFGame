/*
* 文件        : SocketEngine.h
* 版本        : 1.0
* 描述        : 定时器引擎接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/21/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件引用
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "QueueService.h"

//类说明
class CTimerEngine;

//////////////////////////////////////////////////////////////////////////

//定时器线程
class CTimerThread : public CServiceThread
{
	//函数定义
public:
	//构造函数
	CTimerThread(void);
	//析构函数
	virtual ~CTimerThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(CTimerEngine * pTimerEngine, DWORD dwTimerSpace);

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();

	//变量定义
protected:
	HANDLE								m_hEvent;						//超时用事件
	DWORD								m_dwTimerSpace;					//时间间隔
	CTimerEngine						* m_pTimerEngine;				//定时器引擎
};

//////////////////////////////////////////////////////////////////////////

//定时器子项
struct tagTimerItem
{
	WORD								wTimerID;						//定时器 ID
	DWORD								dwElapse;						//定时时间
	DWORD								dwTimeLeave;					//倒计时间
	DWORD								dwRepeatTimes;					//重复次数
	WPARAM								wBindParam;						//绑定参数
};

//定时器子项数组定义
typedef std::unordered_map<WORD, tagTimerItem *> CTimerItemMap;

//////////////////////////////////////////////////////////////////////////

//定时器引擎
class CTimerEngine
{
	friend class CTimerThread;

	//函数定义
public:
	//构造函数
	CTimerEngine(void);
	//析构函数
	virtual ~CTimerEngine(void);

	//接口函数
public:
	//设置定时器，误差0.003秒
	virtual bool SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam);
	//删除定时器
	virtual bool KillTimer(WORD wTimerID);
	//删除定时器
	virtual bool KillAllTimer();

	//管理接口
public:
	//开始服务
	virtual bool BeginService();
	//停止服务
	virtual bool EndService();
	//设置接口
	virtual void SetTimerEngineSink(CQueueServiceBase *pQueueServiceBase);

	//内部函数
private:
	//定时器通知
	void OnTimerThreadSink();

	//配置定义
protected:
	DWORD								m_dwTimerSpace;					//时间间隔
	UINT								m_nTimerResolution;				//时间精度

	//状态变量
protected:
	bool								m_bService;						//运行标志
	DWORD								m_dwTimePass;					//经过时间
	DWORD								m_dwTimeLeave;					//倒计时间
	CTimerItemMap						m_TimerItemFree;				//空闲数组
	CTimerItemMap						m_TimerItemActive;				//活动数组

	//组件变量
protected:
	CThreadLock							m_ThreadLock;					//线程锁
	CTimerThread						m_TimerThread;					//定时器线程
	CQueueServiceEvent					m_AttemperEvent;				//通知组件
};

//////////////////////////////////////////////////////////////////////////
