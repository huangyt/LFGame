#include "LFCommon.h"
#include "LFAttemperEngineSink.h"
#include "LFSessionManager.h"


CLFAttemperEngineSink::CLFAttemperEngineSink(void)
{
}

CLFAttemperEngineSink::~CLFAttemperEngineSink(void)
{
}

//配置函数
void CLFAttemperEngineSink::InitAttemperSink(CQueueService *pDataBaseService)
{
	m_DataBaseEvent.SetQueueService(pDataBaseService);
}

//调度模块启动
bool CLFAttemperEngineSink::BeginService()
{
	sLogEngine.Log(Level_Normal, "登录调度模块启动...");
	return true;
}

//调度模块关闭
bool CLFAttemperEngineSink::EndService()
{
	sGameServer.GetSessionManager()->DeleteAllSession();

	sLogEngine.Log(Level_Normal, "登录调度模块关闭...");
	return true;
}

//其他事件处理接口
bool CLFAttemperEngineSink::OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	return true;
}

//------------------------------------------------------------------------------
//定时器事件
bool CLFAttemperEngineSink::OnEventTimer(WORD wTimerID, WPARAM wBindParam)
{
	return true;
}

//数据库事件
bool CLFAttemperEngineSink::OnEventDataBase(NTY_DataBaseEvent * pDataBaseEvent)
{
	sGameServer.GetSessionManager()->ReceiveDataBaseEvent(pDataBaseEvent->dwSocketID, pDataBaseEvent->wRequestID, pDataBaseEvent->wDataSize, pDataBaseEvent->pDataBuffer);
	return true;
}

//网络应答事件
bool CLFAttemperEngineSink::OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent)
{
	sGameServer.GetSessionManager()->AddSession(pSocketAcceptEvent->dwSocketID, pSocketAcceptEvent->dwClientIP);
	return true;
}

//网络读取事件
bool CLFAttemperEngineSink::OnEventSocketRead(NTY_SocketReadEvent * pSocketReadEvent)
{
	return sGameServer.GetSessionManager()->ReceiveSocketReadEvent(pSocketReadEvent->dwSocketID, pSocketReadEvent->dwCommand, pSocketReadEvent->wDataSize, pSocketReadEvent->pDataBuffer);
}

//网络关闭事件
bool CLFAttemperEngineSink::OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent)
{
	sGameServer.GetSessionManager()->DeleteSession(pSocketCloseEvent->dwSocketID, pSocketCloseEvent->dwClientIP);
	return true;
}