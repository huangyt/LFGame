/*
* �ļ�        : AttemperEngine.h
* �汾        : 1.0
* ����        : ��������ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/20/2010  1.0        ���           ����
* 
*/
#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"
#include "SocketEngine.h"

//////////////////////////////////////////////////////////////////////////

//����ģ�鹳�ӽӿ�
class CAttemperEngineSink
{
	//����ӿ�
public:
	//����ģ������
	virtual bool BeginService()=NULL;
	//����ģ��ر�
	virtual bool EndService()=NULL;
	//�¼�����ӿ�
	virtual bool OnAttemperEvent(WORD wIdentifier, void * pBuffer, WORD wDataSize)=NULL;

	//�¼��ӿ�
public:
	//��ʱ���¼�
	virtual bool OnEventTimer(WORD wTimerID, WPARAM wBindParam)=NULL;
	//���ݿ��¼�
	virtual bool OnEventDataBase(NTY_DataBaseEvent * pDataBaseEvent)=NULL;
	//����Ӧ���¼�
	virtual bool OnEventSocketAccept(NTY_SocketAcceptEvent * pSocketAcceptEvent)=NULL;
	//�����ȡ�¼�
	virtual bool OnEventSocketRead(NTY_SocketReadEvent * pSocketReadEvent)=NULL;
	//����ر��¼�
	virtual bool OnEventSocketClose(NTY_SocketCloseEvent * pSocketCloseEvent)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//���ȹ�����
class CAttemperEngine : public CQueueServiceSink
{
	//��������
public:
	//���캯��
	CAttemperEngine(void);
	//��������
	~CAttemperEngine(void);

	//����ӿ�
public:
	//��������
	bool BeginService();
	//ֹͣ����
	bool EndService();
	//��������
	void SetSocketEngine(CTCPSocketEngine *pTCPSocketEngine);
	//ע�ṳ��
	void SetAttemperEngineSink(CAttemperEngineSink *pAttemperEngineSink);
	//��ȡ�ӿ�
	CQueueService* GetQueueService();

	//���нӿ�
public:
	//�����ӿ�
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//���ı���
protected:
	bool							m_bService;							//���б�־
	CQueueService					m_RequestQueueService;				//���ж���

	//�ӿڱ���
protected:
	CTCPSocketEngine				* m_pTCPSocketEngine;				//��������
	CAttemperEngineSink				* m_pAttemperEngineSink;			//�ҽӽӿ�
};
