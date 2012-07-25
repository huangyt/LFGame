/*
* �ļ�        : AsynchronismEngine.h
* �汾        : 1.0
* ����        : �첽����ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/21/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "DataStorage.h"

//////////////////////////////////////////////////////////////////////////

//�첽���湳�ӽӿ�
class CAsynchronismEngineSink
{
public:
	//�����¼�
	virtual bool OnAsynchronismEngineStart()=NULL;
	//ֹͣ�¼�
	virtual bool OnAsynchronismEngineStop()=NULL;
	//�첽����
	virtual bool OnAsynchronismRequest(WORD wRequestID, void * pBuffer, WORD wDataSize)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//��˵��
class CAsynchronismEngine;
typedef class std::unordered_set<CAsynchronismEngineSink *> CAsynchronismEngineSinkSet;

//////////////////////////////////////////////////////////////////////////

//��Ϣ����
struct stMsg
{
	WPARAM _wParam;
	LPARAM _lParam;
};
typedef std::queue<stMsg*>	MsgQueue;

//��Ϣ�߳�
class CMessageThread : public CServiceThread
{
	//��������
public:
	//���캯��
	CMessageThread();
	//��������
	virtual ~CMessageThread();

	//���ܺ���
public:
	void PostMessage(WPARAM wParam, LPARAM lParam);
	//ֹͣ�߳�
	virtual bool StopThread(DWORD dwWaitSeconds=INFINITE);

	//�¼�����
private:
	//��ʼ�¼�
	virtual bool OnThreadStratEvent();
	//ֹͣ�¼�
	virtual bool OnThreadStopEvent();

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();

	//��Ϣӳ��
protected:
	//������Ϣ
	bool OnAsynRequest(WPARAM wParam, LPARAM lParam);

	//��������
public:
	CAsynchronismEngine				*m_pAsynchronismEngine;			//�첽����
	MsgQueue						m_MsgQueue;						//��Ϣ����
	HANDLE							m_hMsgEvent;					//��Ϣ�¼�
	bool							m_bRunning;						//��Ϣѭ����־
	CThreadLock						m_MsgLock;						//��Ϣ������
};

//////////////////////////////////////////////////////////////////////////

//�첽����
class CAsynchronismEngine
{
	friend class CMessageThread;

	//��������
public:
	//���캯��
	CAsynchronismEngine(void);
	//��������
	~CAsynchronismEngine(void);

	//����ӿ�
public:
	//��������
	bool BeginService();
	//ֹͣ����
	bool EndService();
	//��������
	bool InsertRequest(WORD wRequestID, void * const pBuffer, WORD wDataSize, CAsynchronismEngineSink *pAsynchronismEngineSink);

	//���ܽӿ�
public:
	//ע�ṳ��
	bool RegisterAsynchronismEngineSink(CAsynchronismEngineSink * pAsynchronismEngineSink);
	//ȡ��ע��
	bool UnRegisterAsynchronismEngineSink(CAsynchronismEngineSink * pAsynchronismEngineSink);

	//�̺߳���
protected:
	//�߳�����
	bool OnMessageThreadStart();
	//�߳�ֹͣ
	bool OnMessageThreadStop();
	//�첽����
	void OnAsynchronismRequest(WORD wRequestID, CAsynchronismEngineSink * pAsynchronismEngineSink);

	//�ں˱���
protected:
	bool							m_bService;							//�����־
	BYTE							m_cbBuffer[MAX_QUEUE_PACKET];		//���ݻ���
	CAsynchronismEngineSinkSet		m_AsynchronismEngineSinkSet;		//�첽����

	//�������
protected:
	CThreadLock						m_ThreadLock;						//�߳�ͬ��
	CDataStorage					m_DataStorage;						//���ݴ洢
	CMessageThread					m_MessageThread;					//�߳����
};

//////////////////////////////////////////////////////////////////////////