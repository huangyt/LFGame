#include "LFCommon.h"
#include "LFAsyncEngineSink.h"


CLFAsyncEngineSink::CLFAsyncEngineSink(void)
{
}

CLFAsyncEngineSink::~CLFAsyncEngineSink(void)
{
}

//启动事件
bool CLFAsyncEngineSink::OnAsynchronismEngineStart()
{
	sLogEngine.Log(Level_Normal, "异步调度引擎启动...");
	return true;
}

//停止事件
bool CLFAsyncEngineSink::OnAsynchronismEngineStop()
{
	sLogEngine.Log(Level_Normal, "异步调度引擎关闭...");
	return true;
}

//异步请求
bool CLFAsyncEngineSink::OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize)
{
	return true;
}