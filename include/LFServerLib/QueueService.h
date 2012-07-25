/*
* �ļ�        : QueueService.h
* �汾        : 1.0
* ����        : ���з���ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        ���           ����
* 
*/
#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "DataStorage.h"

//////////////////////////////////////////////////////////////////////////

//���ݶ����๳�ӽӿ�
class CQueueServiceSink
{
public:
	//֪ͨ�ص�����
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)=NULL;
};

//���ݶ��нӿ�
class CQueueServiceBase
{
public:
	virtual LONG GetUnCompleteCount()=NULL;
	//��������
	virtual bool AddToQueue(WORD wIdentifier, void * const pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//�����߳���
class CQueueServiceThread : public CServiceThread
{
	//��Ա����
	friend class CQueueService;

	//���ö���
protected:
	HANDLE							m_hCompletionPort;					//��ɶ˿�

	//��������
private:
	BYTE							m_cbBuffer[MAX_QUEUE_PACKET];		//���ջ���

	//��������
protected:
	//���캯��
	CQueueServiceThread(void);
	//��������
	virtual ~CQueueServiceThread(void);

	//���ܺ���
public:
	//���ú���
	bool InitThread(HANDLE hCompletionPort);
	//ȡ������
	bool UnInitThread();

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//���ݶ�����
class CQueueService : public CQueueServiceBase
{
	friend class CQueueServiceThread;

	//��������
public:
	//���캯��
	CQueueService(void);
	//��������
	virtual ~CQueueService(void);

	//���нӿ�
public:
	//��������
	virtual bool AddToQueue(WORD wIdentifier, void * const pBuffer, WORD wDataSize);
	//�õ�δ���������Ŀ
	virtual LONG GetUnCompleteCount();

	//����ӿ�
public:
	//��ʼ����
	bool BeginService();
	//ֹͣ����
	bool EndService();
	//���ýӿ�
	void SetQueueServiceSink(CQueueServiceSink *pQueueServiceSink);
	//������Ϣ
	bool GetBurthenInfo(tagBurthenInfo & BurthenInfo);

	//��������
private:
	//��ȡ����
	bool GetData(tagDataHead & DataHead, void * pBuffer, WORD wBufferSize);
	//������Ϣ
	void OnQueueServiceThread(const tagDataHead & DataHead, void * pBuffer, WORD wDataSize);

	//��������
protected:
	bool							m_bService;							//�����־
	HANDLE							m_hCompletionPort;					//��ɶ˿�
	CQueueServiceSink				*m_pQueueServiceSink;				//�ص��ӿ�
	LONG							m_lUnComplete;
	//�������
protected:
	CThreadLock						m_ThreadLock;						//�߳���
	CDataStorage					m_DataStorage;						//���ݴ洢
	CQueueServiceThread				m_QueueServiceThread;				//�����߳�
};

//////////////////////////////////////////////////////////////////////////

//���ݶ����¼�
class CQueueServiceEvent
{
	//��������
public:
	//���캯��
	CQueueServiceEvent();
	//��������
	virtual ~CQueueServiceEvent();

	//������
public:
	//���ýӿ�
	void SetQueueService(CQueueServiceBase *pQueueService);

	//֪ͨ����
public:
	//��ʱ���¼�
	bool PostTimerEvent(WORD wTimerID, WPARAM wBindParam);
	//���ݿ��¼�
	bool PostDataBaseEvent(WORD wRequestID, DWORD dwSocketID, const void * pDataBuffer, WORD wDataSize);
	//����Ӧ���¼�
	bool PostSocketAcceptEvent(DWORD dwSocketID, DWORD dwClientIP);
	//�����ȡ�¼�
	bool PostSocketReadEvent(DWORD dwSocketID, DWORD dwCommand, const void * pDataBuffer, WORD wDataSize);
	//����ر��¼�
	bool PostSocketCloseEvent(DWORD dwSocketID, DWORD dwClientIP, DWORD dwConnectSecond);

	LONG GetUnCompleteCount();

	//��������
public:
	CThreadLock						m_BufferLock;						//ͬ������
	CQueueServiceBase				*m_pQueueService;					//���нӿ�
	BYTE							m_cbBuffer[MAX_QUEUE_PACKET];		//���ջ���
};

//////////////////////////////////////////////////////////////////////////
