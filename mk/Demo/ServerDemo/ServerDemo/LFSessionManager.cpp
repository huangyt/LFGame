#include "LFCommon.h"
#include "LFSessionManager.h"
#include "LFSession.h"

CLFSessionManager::CLFSessionManager(void)
{
}

CLFSessionManager::~CLFSessionManager(void)
{
}

void CLFSessionManager::DeleteAllSession()
{
	for(SessionMapIter iter = m_SessionMap.begin(); iter != m_SessionMap.end();)
	{
		iter->second->Uninitialize();
		iter->second->Release();

		sLogEngine.Log(Level_Normal, "强制断开所有客户端");

		iter = m_SessionMap.erase(iter);
	}
}

void CLFSessionManager::AddSession(DWORD dwSocketID, DWORD dwClientIP)
{
	CLFSession	*pSession = m_SessionFactory.GetFreeObject();
	pSession->Initialize(dwSocketID, dwClientIP);
	m_SessionFactory.InsertActiveObject(pSession);

 	m_SessionMap.insert(std::make_pair(dwSocketID, pSession));

	sLogEngine.Log(Level_Normal, "客户端连接: %s SocketID: %d", FormatIP(dwClientIP).c_str(), dwSocketID);
}

void CLFSessionManager::DeleteSession(DWORD dwSocketID, DWORD dwClientIP)
{
 	SessionMapIter iter = m_SessionMap.find(dwSocketID);
 	if(iter != m_SessionMap.end())
 	{
		iter->second->Uninitialize();
 		iter->second->Release();
 		m_SessionMap.erase(iter);
 
 		sLogEngine.Log(Level_Normal, "客户端断开: %s", FormatIP(dwClientIP).c_str());
 	}
}

void CLFSessionManager::ReceiveTimerEvent(WORD wTimerID, WPARAM wBindParam)
{
	for(SessionMapIter iter = m_SessionMap.begin(); iter != m_SessionMap.end(); ++iter)
	{
		iter->second->ReceiveTimerEvent(wTimerID, wBindParam);
	}
}

void CLFSessionManager::ReceiveDataBaseEvent(DWORD dwSocketID, WORD wRequestID, WORD wDataSize, void* pDataBuffer)
{
	SessionMapIter iter = m_SessionMap.find(dwSocketID);
	if(iter != m_SessionMap.end())
	{
		if(iter->second)
		{
			iter->second->ReceiveDataBaseEvent(wRequestID, wDataSize, pDataBuffer);
		}
	}
}

bool CLFSessionManager::ReceiveSocketReadEvent(DWORD dwSocketID, DWORD dwCommand, WORD wDataSize, void* pDataBuffer)
{
	SessionMapIter iter = m_SessionMap.find(dwSocketID);
	if(iter != m_SessionMap.end())
	{
		if(iter->second)
		{
			return iter->second->ReceiveSocketReadEvent(dwCommand, wDataSize, pDataBuffer);
		}
	}
	return false;
}