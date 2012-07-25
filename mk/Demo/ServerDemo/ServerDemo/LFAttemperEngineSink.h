#pragma once

#include "AttemperEngine.h"

class CLFAttemperEngineSink : public CAttemperEngineSink
{
public:
	CLFAttemperEngineSink(void);
	~CLFAttemperEngineSink(void);

	//管理接口
public:
	//配置函数
	void InitAttemperSink(CQueueService *pDataBaseService);

	//调度模块启动
	virtual bool BeginService();

	//调度模块关闭
	virtual bool EndService();

	//其他事件处理接口
	virtual bool OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//事件接口
public:
	//定时器事件
	virtual bool OnEventTimer(WORD wTimerID, WPARAM wBindParam);

	//数据库事件
	virtual bool OnEventDataBase(NTY_DataBaseEvent * pDataBaseEvent);

	//网络应答事件
	virtual bool OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent);

	//网络读取事件
	virtual bool OnEventSocketRead(NTY_SocketReadEvent * pSocketReadEvent);

	//网络关闭事件
	virtual bool OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent);

private:
	CQueueServiceEvent				m_DataBaseEvent;					//数据库通知
};

