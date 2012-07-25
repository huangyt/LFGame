#include "LFCommon.h"
#include "LFAttemperEngineSink.h"
#include "LFSessionManager.h"


CLFAttemperEngineSink::CLFAttemperEngineSink(void)
{
}

CLFAttemperEngineSink::~CLFAttemperEngineSink(void)
{
}

//���ú���
void CLFAttemperEngineSink::InitAttemperSink(CQueueService *pDataBaseService)
{
	m_DataBaseEvent.SetQueueService(pDataBaseService);
}

//����ģ������
bool CLFAttemperEngineSink::BeginService()
{
	sLogEngine.Log(Level_Normal, "��¼����ģ������...");
	return true;
}

//����ģ��ر�
bool CLFAttemperEngineSink::EndService()
{
	sGameServer.GetSessionManager()->DeleteAllSession();

	sLogEngine.Log(Level_Normal, "��¼����ģ��ر�...");
	return true;
}

//�����¼�����ӿ�
bool CLFAttemperEngineSink::OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	return true;
}

//------------------------------------------------------------------------------
//��ʱ���¼�
bool CLFAttemperEngineSink::OnEventTimer(WORD wTimerID, WPARAM wBindParam)
{
	return true;
}

//���ݿ��¼�
bool CLFAttemperEngineSink::OnEventDataBase(NTY_DataBaseEvent * pDataBaseEvent)
{
	sGameServer.GetSessionManager()->ReceiveDataBaseEvent(pDataBaseEvent->dwSocketID, pDataBaseEvent->wRequestID, pDataBaseEvent->wDataSize, pDataBaseEvent->pDataBuffer);
	return true;
}

//����Ӧ���¼�
bool CLFAttemperEngineSink::OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent)
{
	sGameServer.GetSessionManager()->AddSession(pSocketAcceptEvent->dwSocketID, pSocketAcceptEvent->dwClientIP);
	return true;
}

//�����ȡ�¼�
bool CLFAttemperEngineSink::OnEventSocketRead(NTY_SocketReadEvent * pSocketReadEvent)
{
	return sGameServer.GetSessionManager()->ReceiveSocketReadEvent(pSocketReadEvent->dwSocketID, pSocketReadEvent->dwCommand, pSocketReadEvent->wDataSize, pSocketReadEvent->pDataBuffer);
}

//����ر��¼�
bool CLFAttemperEngineSink::OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent)
{
	sGameServer.GetSessionManager()->DeleteSession(pSocketCloseEvent->dwSocketID, pSocketCloseEvent->dwClientIP);
	return true;
}