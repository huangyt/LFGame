#include "LFCommon.h"
#include "LFAsyncEngineSink.h"


CLFAsyncEngineSink::CLFAsyncEngineSink(void)
{
}

CLFAsyncEngineSink::~CLFAsyncEngineSink(void)
{
}

//�����¼�
bool CLFAsyncEngineSink::OnAsynchronismEngineStart()
{
	sLogEngine.Log(Level_Normal, "�첽������������...");
	return true;
}

//ֹͣ�¼�
bool CLFAsyncEngineSink::OnAsynchronismEngineStop()
{
	sLogEngine.Log(Level_Normal, "�첽��������ر�...");
	return true;
}

//�첽����
bool CLFAsyncEngineSink::OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize)
{
	return true;
}