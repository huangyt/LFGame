#pragma once

#include "AsynchronismEngine.h"

class CLFAsyncEngineSink : public CAsynchronismEngineSink
{
public:
	CLFAsyncEngineSink(void);
	~CLFAsyncEngineSink(void);

public:
	//�����¼�
	virtual bool OnAsynchronismEngineStart();

	//ֹͣ�¼�
	virtual bool OnAsynchronismEngineStop();

	//�첽����
	virtual bool OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize);
};

