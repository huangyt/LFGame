/*
* �ļ�        : SocketEngine.h
* �汾        : 1.0
* ����        : ��ʱ������ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/21/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "QueueService.h"

//��˵��
class CTimerEngine;

//////////////////////////////////////////////////////////////////////////

//��ʱ���߳�
class CTimerThread : public CServiceThread
{
	//��������
public:
	//���캯��
	CTimerThread(void);
	//��������
	virtual ~CTimerThread(void);

	//���ܺ���
public:
	//���ú���
	bool InitThread(CTimerEngine * pTimerEngine, DWORD dwTimerSpace);

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();

	//��������
protected:
	HANDLE								m_hEvent;						//��ʱ���¼�
	DWORD								m_dwTimerSpace;					//ʱ����
	CTimerEngine						* m_pTimerEngine;				//��ʱ������
};

//////////////////////////////////////////////////////////////////////////

//��ʱ������
struct tagTimerItem
{
	WORD								wTimerID;						//��ʱ�� ID
	DWORD								dwElapse;						//��ʱʱ��
	DWORD								dwTimeLeave;					//����ʱ��
	DWORD								dwRepeatTimes;					//�ظ�����
	WPARAM								wBindParam;						//�󶨲���
};

//��ʱ���������鶨��
typedef std::unordered_map<WORD, tagTimerItem *> CTimerItemMap;

//////////////////////////////////////////////////////////////////////////

//��ʱ������
class CTimerEngine
{
	friend class CTimerThread;

	//��������
public:
	//���캯��
	CTimerEngine(void);
	//��������
	virtual ~CTimerEngine(void);

	//�ӿں���
public:
	//���ö�ʱ�������0.003��
	virtual bool SetTimer(WORD wTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM wParam);
	//ɾ����ʱ��
	virtual bool KillTimer(WORD wTimerID);
	//ɾ����ʱ��
	virtual bool KillAllTimer();

	//����ӿ�
public:
	//��ʼ����
	virtual bool BeginService();
	//ֹͣ����
	virtual bool EndService();
	//���ýӿ�
	virtual void SetTimerEngineSink(CQueueServiceBase *pQueueServiceBase);

	//�ڲ�����
private:
	//��ʱ��֪ͨ
	void OnTimerThreadSink();

	//���ö���
protected:
	DWORD								m_dwTimerSpace;					//ʱ����
	UINT								m_nTimerResolution;				//ʱ�侫��

	//״̬����
protected:
	bool								m_bService;						//���б�־
	DWORD								m_dwTimePass;					//����ʱ��
	DWORD								m_dwTimeLeave;					//����ʱ��
	CTimerItemMap						m_TimerItemFree;				//��������
	CTimerItemMap						m_TimerItemActive;				//�����

	//�������
protected:
	CThreadLock							m_ThreadLock;					//�߳���
	CTimerThread						m_TimerThread;					//��ʱ���߳�
	CQueueServiceEvent					m_AttemperEvent;				//֪ͨ���
};

//////////////////////////////////////////////////////////////////////////
