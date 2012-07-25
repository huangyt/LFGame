#pragma once

#include "DataBaseEngine.h"

class CLFDataBaseSink : public CDataBaseSink
{
public:
	CLFDataBaseSink(void);
	~CLFDataBaseSink(void);

public:
	//��ʼ��
	void InitDataBaseSink(CQueueService *pAttemperServer);

	CDataBase* GetLoginDataBase(void);

	//���ݿ�ģ������
	virtual bool BeginService(void);

	//���ݿ�ģ��ر�
	virtual bool EndService(void);

	//���ݲ�������
	virtual bool OnDataBaseRequest(const NTY_DataBaseEvent & DataBaseEvent);

private:
	CQueueServiceEvent				m_AttemperEvent;
	CDataBase						m_LoginDataBase;
};

