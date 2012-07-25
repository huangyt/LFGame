#pragma once

#include "Factory.h"
#include "ByteBuffer.h"

class CLFSession
{
	typedef CObjectFactory<CLFSession> CSessionFactory;
	friend class CSessionFactory;

 	typedef bool (CLFSession::*Handler)(void);
 	typedef std::unordered_map<DWORD, Handler> CCommandMap;
public:
	//设置初始化信息
	void Initialize(DWORD dwSocketID, DWORD dwClientIP);

	void Uninitialize(void);

	//回收Session
	void Release(){m_pSessionFactory->ReleaseObject(this);}

public:
	//收到定时器消息
	void ReceiveTimerEvent(WORD wTimerID, WPARAM wBindParam);

	//收到数据库异步回调消息
	void ReceiveDataBaseEvent(WORD wRequestID, WORD wDataSize, void* pDataBuffer);

	//收到普通的客户端消息
	bool ReceiveSocketReadEvent(DWORD dwCommand, WORD wDataSize, void* pDataBuffer);

private:
	CLFSession(CSessionFactory* pFactory);
	~CLFSession(void){};

	bool SendData(DWORD dwCommand, void * pData, WORD wDataSize);

	//CMD响应函数
	bool HandleLogonChallenge();

private:
	CSessionFactory	*m_pSessionFactory;

	DWORD			m_dwSocketID;
	DWORD			m_dwClientIP;

	CByteBuffer		m_SendBuffer;
	CByteBuffer		m_RecvBuffer;

	CCommandMap		m_CommandMap;
};