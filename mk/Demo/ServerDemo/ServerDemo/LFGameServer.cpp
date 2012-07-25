#include "LFCommon.h"
#include "LFGameServer.h"

CLFGameServer::CLFGameServer()
	:m_bService(false)
{
}

CLFGameServer::~CLFGameServer()
{
	EndService();
}

CTimerEngine* CLFGameServer::GetTimerEngine()
{
	return &m_TimerEngine;
}

CTCPSocketEngine* CLFGameServer::GetTCPSocketEngine()
{
	return &m_TCPSocketEngine;
}

CLFSessionManager* CLFGameServer::GetSessionManager()
{
	return &m_SessionManager;
}

CDataBase* CLFGameServer::GetLoginDatabase()
{
	return m_LoginDataBaseSink.GetLoginDataBase();
}

bool CLFGameServer::BegineService()
{
	sLogEngine.Log(Level_Normal, "��¼����ʼ����...");

	//�ж�״̬
	if (m_bService==true)
	{
		return true;
	}

	//�����
	m_AttemperEngine.SetSocketEngine(&m_TCPSocketEngine);

	m_TimerEngine.SetTimerEngineSink(m_AttemperEngine.GetQueueService());
	m_TCPSocketEngine.SetSocketEngineSink(m_AttemperEngine.GetQueueService());

	m_DataBaseEngine.SetDataBaseSink(&m_LoginDataBaseSink);
	m_AttemperEngine.SetAttemperEngineSink(&m_AttemperEngineSink);
	m_AsynchronismEngine.RegisterAsynchronismEngineSink(&m_AsyncEngineSink);

	m_LoginDataBaseSink.InitDataBaseSink(m_AttemperEngine.GetQueueService());
	m_AttemperEngineSink.InitAttemperSink(m_DataBaseEngine.GetQueueService());

	m_TCPSocketEngine.SetServicePort(6700);
	m_TCPSocketEngine.SetMaxSocketItem(2000);
	m_TCPSocketEngine.SetNoDelayMod(true);

	//�����������
	if (m_TimerEngine.BeginService()==false) return false;
	if (m_DataBaseEngine.BeginService()==false) return false;
	if (m_AsynchronismEngine.BeginService()==false) return false;
	if (m_AttemperEngine.BeginService()==false) return false;
	if (m_TCPSocketEngine.BeginService()==false) return false;

	//���ñ���
	m_bService=true;

	sLogEngine.Log(Level_Normal, "��¼���������ɹ�...");
	return true;
}

bool CLFGameServer::EndService()
{
	//�ж�״̬
	if (m_bService==false)
	{
		return true;
	}

	//���ñ���
	m_bService=false;

	//ֹͣ��������
	m_AttemperEngine.EndService();
	m_DataBaseEngine.EndService();
	m_TimerEngine.EndService();
	m_TCPSocketEngine.EndService();
	m_AsynchronismEngine.EndService();

	sLogEngine.Log(Level_Normal, "��¼����ر�...");
	return true;
}