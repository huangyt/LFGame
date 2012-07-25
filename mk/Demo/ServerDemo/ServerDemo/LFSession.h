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
	//���ó�ʼ����Ϣ
	void Initialize(DWORD dwSocketID, DWORD dwClientIP);

	void Uninitialize(void);

	//����Session
	void Release(){m_pSessionFactory->ReleaseObject(this);}

public:
	//�յ���ʱ����Ϣ
	void ReceiveTimerEvent(WORD wTimerID, WPARAM wBindParam);

	//�յ����ݿ��첽�ص���Ϣ
	void ReceiveDataBaseEvent(WORD wRequestID, WORD wDataSize, void* pDataBuffer);

	//�յ���ͨ�Ŀͻ�����Ϣ
	bool ReceiveSocketReadEvent(DWORD dwCommand, WORD wDataSize, void* pDataBuffer);

private:
	CLFSession(CSessionFactory* pFactory);
	~CLFSession(void){};

	bool SendData(DWORD dwCommand, void * pData, WORD wDataSize);

	//CMD��Ӧ����
	bool HandleLogonChallenge();

private:
	CSessionFactory	*m_pSessionFactory;

	DWORD			m_dwSocketID;
	DWORD			m_dwClientIP;

	CByteBuffer		m_SendBuffer;
	CByteBuffer		m_RecvBuffer;

	CCommandMap		m_CommandMap;
};