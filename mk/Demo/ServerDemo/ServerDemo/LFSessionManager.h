#pragma once

class CLFSession;

//����ֻ��ͬһ���߳��ڵ��ã����Բ���Ҫ��
class CLFSessionManager
{
	typedef std::unordered_map<DWORD, CLFSession*>				SessionMap;
	typedef std::unordered_map<DWORD, CLFSession*>::iterator	SessionMapIter;
	typedef CObjectFactory<CLFSession>							CSessionFactory;
public:
	CLFSessionManager(void);
	~CLFSessionManager(void);

public:
	void DeleteAllSession(void);

	void AddSession(DWORD dwSocketID, DWORD dwClientIP);

	void DeleteSession(DWORD dwSocketID, DWORD dwClientIP);

	void ReceiveTimerEvent(WORD wTimerID, WPARAM wBindParam);

	void ReceiveDataBaseEvent(DWORD dwSocketID, WORD wRequestID, WORD wDataSize, void* pDataBuffer);

	bool ReceiveSocketReadEvent(DWORD dwSocketID, DWORD dwCommand, WORD wDataSize, void* pDataBuffer);
private:
	SessionMap		m_SessionMap;
	CSessionFactory	m_SessionFactory;
};
