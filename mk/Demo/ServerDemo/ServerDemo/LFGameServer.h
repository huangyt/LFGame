#pragma once

#include "LFDataBaseSink.h"
#include "LFAttemperEngineSink.h"
#include "LFAsyncEngineSink.h"

#include "LFSessionManager.h"

class CLFGameServer : public Singleton<CLFGameServer>
{
	friend class Singleton<CLFGameServer>;
public:
	CLFGameServer(void);
	~CLFGameServer(void);

public:
	CTimerEngine* GetTimerEngine(void);

	CTCPSocketEngine* GetTCPSocketEngine(void);

	CLFSessionManager* GetSessionManager(void);

	CDataBase* GetLoginDatabase(void);

public:
	bool BegineService(void);

	bool EndService(void);

private:
	bool					m_bService;

	//base member
	CTimerEngine			m_TimerEngine;
	CDataBaseEngine			m_DataBaseEngine;
	CAttemperEngine			m_AttemperEngine;
	CTCPSocketEngine		m_TCPSocketEngine;
	CAsynchronismEngine		m_AsynchronismEngine;

	//other
	CLFSessionManager		m_SessionManager;

	//Sink
	CLFDataBaseSink			m_LoginDataBaseSink;
	CLFAttemperEngineSink	m_AttemperEngineSink;
	CLFAsyncEngineSink		m_AsyncEngineSink;
};

