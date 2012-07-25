/*
* �ļ�        : CLogEngine.h
* �汾        : 1.0
* ����        : ��־����ӿڶ���
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/26/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�����
#include <iostream>
#include "Singleton.h"
#include "QueueService.h"

//////////////////////////////////////////////////////////////////////////

//����ȼ�
enum enLogLevel
{
	Level_Normal					=0,									//��ͨ��Ϣ
	Level_Warning					=1,									//������Ϣ
	Level_Exception					=2,									//�쳣��Ϣ
	Level_Debug						=3,									//������Ϣ
};

#define EVENT_LEVEL_COUNT			4

//////////////////////////////////////////////////////////////////////////

//��־������
class CLogEngine :  public CQueueServiceSink, public Singleton<CLogEngine>
{
	friend class Singleton<CLogEngine>;
protected:
	CLogEngine();
	~CLogEngine();

public:
	//��־���
	void Log(enLogLevel logLevel, std::string strLog, ...);
	//���������־
	void ConfigShowFlag(enLogLevel LogLevel, bool bShow);

private:
	//����ص�
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

private:
	//��־����
	CQueueService		m_LogQueueService;
	bool				m_bLogShowFlag[EVENT_LEVEL_COUNT];
};

//////////////////////////////////////////////////////////////////////////