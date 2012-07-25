#pragma once

#include "DataBaseEngine.h"

class CLFDataBaseSink : public CDataBaseSink
{
public:
	CLFDataBaseSink(void);
	~CLFDataBaseSink(void);

public:
	//初始化
	void InitDataBaseSink(CQueueService *pAttemperServer);

	CDataBase* GetLoginDataBase(void);

	//数据库模块启动
	virtual bool BeginService(void);

	//数据库模块关闭
	virtual bool EndService(void);

	//数据操作处理
	virtual bool OnDataBaseRequest(const NTY_DataBaseEvent & DataBaseEvent);

private:
	CQueueServiceEvent				m_AttemperEvent;
	CDataBase						m_LoginDataBase;
};

