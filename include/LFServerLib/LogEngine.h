/*
* 文件        : CLogEngine.h
* 版本        : 1.0
* 描述        : 日志引擎接口定义
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/26/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件引用
#include <iostream>
#include "Singleton.h"
#include "QueueService.h"

//////////////////////////////////////////////////////////////////////////

//输出等级
enum enLogLevel
{
	Level_Normal					=0,									//普通消息
	Level_Warning					=1,									//警告消息
	Level_Exception					=2,									//异常消息
	Level_Debug						=3,									//调试消息
};

#define EVENT_LEVEL_COUNT			4

//////////////////////////////////////////////////////////////////////////

//日志引擎类
class CLogEngine :  public CQueueServiceSink, public Singleton<CLogEngine>
{
	friend class Singleton<CLogEngine>;
protected:
	CLogEngine();
	~CLogEngine();

public:
	//日志输出
	void Log(enLogLevel logLevel, std::string strLog, ...);
	//设置输出标志
	void ConfigShowFlag(enLogLevel LogLevel, bool bShow);

private:
	//服务回调
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

private:
	//日志队列
	CQueueService		m_LogQueueService;
	bool				m_bLogShowFlag[EVENT_LEVEL_COUNT];
};

//////////////////////////////////////////////////////////////////////////