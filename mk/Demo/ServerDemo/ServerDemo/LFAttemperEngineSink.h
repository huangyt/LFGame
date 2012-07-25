#pragma once

#include "AttemperEngine.h"

class CLFAttemperEngineSink : public CAttemperEngineSink
{
public:
	CLFAttemperEngineSink(void);
	~CLFAttemperEngineSink(void);

	//����ӿ�
public:
	//���ú���
	void InitAttemperSink(CQueueService *pDataBaseService);

	//����ģ������
	virtual bool BeginService();

	//����ģ��ر�
	virtual bool EndService();

	//�����¼�����ӿ�
	virtual bool OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//�¼��ӿ�
public:
	//��ʱ���¼�
	virtual bool OnEventTimer(WORD wTimerID, WPARAM wBindParam);

	//���ݿ��¼�
	virtual bool OnEventDataBase(NTY_DataBaseEvent * pDataBaseEvent);

	//����Ӧ���¼�
	virtual bool OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent);

	//�����ȡ�¼�
	virtual bool OnEventSocketRead(NTY_SocketReadEvent * pSocketReadEvent);

	//����ر��¼�
	virtual bool OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent);

private:
	CQueueServiceEvent				m_DataBaseEvent;					//���ݿ�֪ͨ
};

