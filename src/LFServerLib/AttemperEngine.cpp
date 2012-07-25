#include "AttemperEngine.h"

//////////////////////////////////////////////////////////////////////////

//���캯��
CAttemperEngine::CAttemperEngine(void)
{
	m_bService=false;
	m_pTCPSocketEngine=NULL;
	m_pAttemperEngineSink=NULL;
	return;
}

//��������
CAttemperEngine::~CAttemperEngine(void)
{
}

//��������
bool CAttemperEngine::BeginService()
{
	//�ж�״̬
	if (m_bService==true) 
	{
		return true;
	}

	//��ҽӿ�
	if (m_pAttemperEngineSink==NULL)
	{
		return false;
	}

	//���ö���
	m_RequestQueueService.SetQueueServiceSink(static_cast<CQueueServiceSink*>(this));

	//�������
	if (m_pAttemperEngineSink->BeginService()==false)
	{
		return false;
	}

	//��������
	if (m_RequestQueueService.BeginService()==false)
	{
		return false;
	}

	//���ñ���
	m_bService=true;

	return true;
}

//ֹͣ����
bool CAttemperEngine::EndService()
{
	//���ñ���
	m_bService=false;

	//ֹͣ�������
	m_RequestQueueService.EndService();

	//ֹͣ���
	if (m_pAttemperEngineSink!=NULL)
	{
		m_pAttemperEngineSink->EndService();
	}

	return true;
}

//��������
void CAttemperEngine::SetSocketEngine(CTCPSocketEngine *pTCPSocketEngine)
{
	ASSERT(pTCPSocketEngine!=NULL);
	m_pTCPSocketEngine = pTCPSocketEngine;
}

//ע�ṳ��
void CAttemperEngine::SetAttemperEngineSink(CAttemperEngineSink *pAttemperEngineSink)
{
	//Ч�����
	ASSERT(pAttemperEngineSink!=NULL);

	//��ѯ�ӿ�
	m_pAttemperEngineSink  = pAttemperEngineSink;
}

//��ȡ�ӿ�
CQueueService* CAttemperEngine::GetQueueService()
{
	return &m_RequestQueueService;
}

//���нӿ�
void CAttemperEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	//�ں��¼�
	ASSERT(m_pAttemperEngineSink!=NULL);
	switch (wIdentifier)
	{
	case EVENT_TIMER:			//��ʱ���¼�
		{
			//Ч�����
			ASSERT(wDataSize==sizeof(NTY_TimerEvent));
			if (wDataSize!=sizeof(NTY_TimerEvent)) return;

			//������Ϣ
			NTY_TimerEvent * pTimerEvent=(NTY_TimerEvent *)pBuffer;
			try
			{
				m_pAttemperEngineSink->OnEventTimer(pTimerEvent->wTimerID,pTimerEvent->wBindParam);
			}
			catch (...){ }

			return;
		}
	case EVENT_DATABASE:		//���ݿ��¼�
		{
			//Ч�����
			ASSERT(wDataSize>=sizeof(NTY_DataBaseEvent));
			if (wDataSize<sizeof(NTY_DataBaseEvent)) return;

			//������Ϣ
			NTY_DataBaseEvent * pDataBaseEvent=(NTY_DataBaseEvent *)pBuffer;
			if(pDataBaseEvent->wDataSize == 0)
			{
				pDataBaseEvent->pDataBuffer = NULL;
			}
			else
			{
				pDataBaseEvent->pDataBuffer=pDataBaseEvent+1;
			}
			try
			{
				m_pAttemperEngineSink->OnEventDataBase(pDataBaseEvent);
			}
			catch (...){ }

			return;
		}
	case EVENT_SOCKET_ACCEPT:	//����Ӧ���¼�
		{
			//Ч���С
			ASSERT(wDataSize==sizeof(NTY_SocketAcceptEvent));
			if (wDataSize!=sizeof(NTY_SocketAcceptEvent)) return;

			//������Ϣ
			NTY_SocketAcceptEvent * pSocketAcceptEvent=(NTY_SocketAcceptEvent *)pBuffer;
			bool bSuccess=false;
			try 
			{ 
				bSuccess = m_pAttemperEngineSink->OnEventSocketAccept(pSocketAcceptEvent);
			}
			catch (...)	{ }
			if (bSuccess==false) m_pTCPSocketEngine->CloseSocket(pSocketAcceptEvent->dwSocketID);


			return;
		}
	case EVENT_SOCKET_READ:		//�����ȡ�¼�
		{
			//Ч���С
			NTY_SocketReadEvent * pSocketReadEvent=(NTY_SocketReadEvent *)pBuffer;
			ASSERT(wDataSize>=sizeof(NTY_SocketReadEvent));
			ASSERT(wDataSize==(sizeof(NTY_SocketReadEvent)+pSocketReadEvent->wDataSize));
			if (wDataSize<sizeof(NTY_SocketReadEvent)) return;
			if (wDataSize!=(sizeof(NTY_SocketReadEvent)+pSocketReadEvent->wDataSize)) return;

			//������Ϣ
			bool bSuccess=false;
			try 
			{ 
				if(pSocketReadEvent->wDataSize == 0)
				{
					pSocketReadEvent->pDataBuffer = NULL;
				}
				else
				{
					pSocketReadEvent->pDataBuffer=pSocketReadEvent+1;
				}
				bSuccess=m_pAttemperEngineSink->OnEventSocketRead(pSocketReadEvent);
			}
			catch (...)	{ }
			if (bSuccess==false) m_pTCPSocketEngine->CloseSocket(pSocketReadEvent->dwSocketID);

			return;
		}
	case EVENT_SOCKET_CLOSE:	//����ر��¼�
		{
			//Ч���С
			ASSERT(wDataSize==sizeof(NTY_SocketCloseEvent));
			if (wDataSize!=sizeof(NTY_SocketCloseEvent)) return;

			//������Ϣ
			NTY_SocketCloseEvent * pSocketCloseEvent=(NTY_SocketCloseEvent *)pBuffer;
			bool bSuccess = false;
			try
			{
				bSuccess = m_pAttemperEngineSink->OnEventSocketClose(pSocketCloseEvent);
			}
			catch (...)	{ }
			if (bSuccess==false) m_pTCPSocketEngine->CloseSocket(pSocketCloseEvent->dwSocketID);

			return;
		}
	}

	//�����¼�
	m_pAttemperEngineSink->OnAttemperEvent(wIdentifier, pBuffer, wDataSize); 

	return;
}