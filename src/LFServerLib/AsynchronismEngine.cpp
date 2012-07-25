#include "AsynchronismEngine.h"

#include <iostream>

//////////////////////////////////////////////////////////////////////////

//���캯��
CMessageThread::CMessageThread()
{
	m_pAsynchronismEngine=NULL;
	m_bRunning = true;
	m_hMsgEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//��������
CMessageThread::~CMessageThread()
{
	for(UINT i=0; i<m_MsgQueue.size(); i++)
	{
		SAFE_DELETE(m_MsgQueue.front());
		m_MsgQueue.pop();
	}
	CloseHandle(m_hMsgEvent);
}

//���к���
bool CMessageThread::RepetitionRun()
{
	while(m_bRunning)
	{
		WaitForSingleObject(m_hMsgEvent, INFINITE);

		CThreadLockHandle ThreadLockHandle(&m_MsgLock);
		if(m_MsgQueue.size()>0)
		{
			stMsg *pTemp = m_MsgQueue.front();
			OnAsynRequest(pTemp->_wParam, pTemp->_lParam);
			SAFE_DELETE(pTemp);
			m_MsgQueue.pop();
		}
		if(m_MsgQueue.size()>0)
		{
			SetEvent(m_hMsgEvent);
		}
		ThreadLockHandle.UnLock();
	}
	
	return true;
}

void CMessageThread::PostMessage(WPARAM wParam, LPARAM lParam)
{
	//������Ϣ
	stMsg *pMsg = new stMsg;
	pMsg->_wParam = wParam;
	pMsg->_lParam = lParam;

	CThreadLockHandle ThreadLockHandle(&m_MsgLock);
	m_MsgQueue.push(pMsg);
	SetEvent(m_hMsgEvent);
}

//ֹͣ�߳�
bool CMessageThread::StopThread(DWORD dwWaitSeconds)
{
	m_bRunning = false;
	SetEvent(m_hMsgEvent);

	return CServiceThread::StopThread(dwWaitSeconds);
}

//��ʼ�¼�
bool CMessageThread::OnThreadStratEvent()
{
	ASSERT(m_pAsynchronismEngine!=NULL);
	return m_pAsynchronismEngine->OnMessageThreadStart();
}

//ֹͣ�¼�
bool CMessageThread::OnThreadStopEvent()
{
	ASSERT(m_pAsynchronismEngine!=NULL);
	return m_pAsynchronismEngine->OnMessageThreadStop();
}

//������Ϣ
bool CMessageThread::OnAsynRequest(WPARAM wParam, LPARAM lParam)
{
	//Ч�����
	ASSERT(m_pAsynchronismEngine!=NULL);

	//��ȡ����
	WORD wRequestID=(WORD)wParam;
	CAsynchronismEngineSink * pAsynchronismEngineSink=(CAsynchronismEngineSink *)lParam;

	//������
	ASSERT(pAsynchronismEngineSink!=NULL);
	m_pAsynchronismEngine->OnAsynchronismRequest(wRequestID,pAsynchronismEngineSink);

	return true;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CAsynchronismEngine::CAsynchronismEngine(void)
{
	//���ñ���
	m_bService=false;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));

	//�������
	m_MessageThread.m_pAsynchronismEngine=this;

	return;
}

//��������
CAsynchronismEngine::~CAsynchronismEngine(void)
{
}

//��������
bool CAsynchronismEngine::BeginService()
{
	//�����߳�
	if (m_MessageThread.StartThead()==false) return false;

	//���ñ���
	m_bService=true;

	return true;
}

//ֹͣ����
bool CAsynchronismEngine::EndService()
{
	//���ñ���
	m_bService=false;

	//ֹͣ�߳�
	m_MessageThread.StopThread();

	//��������
	m_DataStorage.RemoveData(false);

	return true;
}

//��������
bool CAsynchronismEngine::InsertRequest(WORD wRequestID, void * const pBuffer, WORD wDataSize, CAsynchronismEngineSink *pAsynchronismEngineSink)
{
	//Ч��״̬
	ASSERT(wDataSize<=sizeof(m_cbBuffer));
	if (m_bService==false) return false;
	if (wDataSize>sizeof(m_cbBuffer)) return false;

	//��ѯ�ӿ�
	ASSERT(pAsynchronismEngineSink!=NULL);
	if (pAsynchronismEngineSink==NULL) return false;

	//��������
	CThreadLockHandle ThreadLockHandle(&m_ThreadLock);
	if (m_DataStorage.AddData(wRequestID,pBuffer,wDataSize)==false) return false;

	//Ͷ����Ϣ
	m_MessageThread.PostMessage(wRequestID,(LPARAM)pAsynchronismEngineSink);

	return true;
}

//ע�ṳ��
bool CAsynchronismEngine::RegisterAsynchronismEngineSink(CAsynchronismEngineSink *pAsynchronismEngineSink)
{
	ASSERT(pAsynchronismEngineSink!=NULL);
	if (pAsynchronismEngineSink==NULL) return false;

	//�����ִ�
	CAsynchronismEngineSinkSet::iterator iter = m_AsynchronismEngineSinkSet.find(pAsynchronismEngineSink);
	if(iter != m_AsynchronismEngineSinkSet.end()) return false;
 
 	//ע�ṳ��
 	m_AsynchronismEngineSinkSet.insert(pAsynchronismEngineSink);

	return true;
}

//ȡ��ע��
bool CAsynchronismEngine::UnRegisterAsynchronismEngineSink(CAsynchronismEngineSink *pAsynchronismEngineSink)
{
	ASSERT(pAsynchronismEngineSink!=NULL);
	if (pAsynchronismEngineSink==NULL) return false;

	//ɾ������
	CAsynchronismEngineSinkSet::iterator iter = m_AsynchronismEngineSinkSet.find(pAsynchronismEngineSink);
	if(iter != m_AsynchronismEngineSinkSet.end())
	{
		m_AsynchronismEngineSinkSet.erase(iter);
		return true;
	}

	return false;
}

//�߳�����
bool CAsynchronismEngine::OnMessageThreadStart()
{
	//�¼�֪ͨ
	bool bSuccsee=true;
	for(CAsynchronismEngineSinkSet::iterator iter = m_AsynchronismEngineSinkSet.begin();
		iter != m_AsynchronismEngineSinkSet.end(); ++iter)
	{
		try
		{
			if ((*iter)->OnAsynchronismEngineStart()==false) bSuccsee=false;
		}
		catch (...)	{ bSuccsee=false; }
	}

	return bSuccsee;
}

//�߳�ֹͣ
bool CAsynchronismEngine::OnMessageThreadStop()
{
	//�¼�֪ͨ
	bool bSuccsee=true;
	for(CAsynchronismEngineSinkSet::iterator iter = m_AsynchronismEngineSinkSet.begin();
		iter != m_AsynchronismEngineSinkSet.end(); ++iter)
	{
		try
		{
			if ((*iter)->OnAsynchronismEngineStop()==false) bSuccsee=false;
		}
		catch (...)	{ bSuccsee=false; }
	}

	return bSuccsee;
}

//�첽����
void CAsynchronismEngine::OnAsynchronismRequest(WORD wRequestID, CAsynchronismEngineSink * pAsynchronismEngineSink)
{
	//������Դ
	CThreadLockHandle ThreadLockHandle(&m_ThreadLock);

	//��ȡ����
	tagDataHead DataHead;
	m_DataStorage.GetData(DataHead,m_cbBuffer,sizeof(m_cbBuffer));

	//��Ϣ����
	try
	{
		ASSERT(DataHead.wIdentifier==wRequestID);
		pAsynchronismEngineSink->OnAsynchronismRequest(wRequestID,m_cbBuffer,DataHead.wDataSize);
	}
	catch (...) {}

	return;
}

//////////////////////////////////////////////////////////////////////////