#pragma once

#include "AsynchronismEngine.h"

class CLFAsyncEngineSink : public CAsynchronismEngineSink
{
public:
	CLFAsyncEngineSink(void);
	~CLFAsyncEngineSink(void);

public:
	//启动事件
	virtual bool OnAsynchronismEngineStart();

	//停止事件
	virtual bool OnAsynchronismEngineStop();

	//异步请求
	virtual bool OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize);
};

