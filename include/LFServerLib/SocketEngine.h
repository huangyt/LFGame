/*
* �ļ�        : SocketEngine.h
* �汾        : 1.0
* ����        : IOCP����ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "QueueService.h"
#include "AuthCrypt.h"

//////////////////////////////////////////////////////////////////////////
//ö�ٶ���

//��������
enum enOperationType
{
	OperationType_Send,				//��������
	OperationType_Recv,				//��������
};

//////////////////////////////////////////////////////////////////////////
//��˵��

class COverLapped;
class CServerSocketRSThread;
class COverLappedSend;
class CTCPSocketEngine;
class CServerSocketItem;
class CSocketAcceptThread;
template <enOperationType OperationType> class CATLOverLapped;

typedef class CATLOverLapped<OperationType_Recv> COverLappedRecv;
typedef std::unordered_set<COverLappedSend *> COverLappedSendSet;
typedef std::unordered_set<COverLappedRecv *> COverLappedRecvSet;

//���Ӷ���ص��ӿ�
class CServerSocketItemSink
{
public:
	//Ӧ����Ϣ
	virtual bool OnSocketAcceptEvent(CServerSocketItem * pServerSocketItem)=NULL;
	//��ȡ��Ϣ
	virtual bool OnSocketReadEvent(DWORD dwCommand, void * pBuffer, WORD wDataSize, CServerSocketItem * pServerSocketItem)=NULL;
	//�ر���Ϣ
	virtual bool OnSocketCloseEvent(CServerSocketItem * pServerSocketItem)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//�ص��ṹ��
class COverLapped
{
	//��������
public:
	//���캯��
	COverLapped(enOperationType OperationType);
	//��������
	virtual ~COverLapped();

	//��Ϣ����
public:
	//��ȡ����
	enOperationType GetOperationType() { return m_OperationType; }

	//��������
public:
	WSABUF							m_WSABuffer;						//����ָ��
	OVERLAPPED						m_OverLapped;						//�ص��ṹ
	const enOperationType			m_OperationType;					//��������
};

//�����ص��ṹ
class COverLappedSend : public COverLapped
{
	//��������
public:
	//���캯��
	COverLappedSend();
	//��������
	virtual ~COverLappedSend();

	//���ݱ���
public:
	BYTE							m_cbBuffer[SOCKET_BUFFER];			//���ݻ���
};

//�ص��ṹģ��
template <enOperationType OperationType> class CATLOverLapped : public COverLapped
{
	//��������
public:
	//���캯��
	CATLOverLapped() : COverLapped(OperationType) {}
	//��������
	virtual ~CATLOverLapped() {}
};

//////////////////////////////////////////////////////////////////////////

//TCP SOCKET ��
class CServerSocketItem
{
	//��������
public:
	//���캯��
	CServerSocketItem(WORD wIndex, CServerSocketItemSink * pServerSocketItemSink);
	//��������
	virtual ~CServerSocketItem(void);

	//��ʶ����
public:
	//��ȡ����
	WORD GetIndex() { return m_wIndex; }
	//��ȡ����
	WORD GetRountID() { return m_wRountID; }
	//��ȡ��ʶ
	DWORD GetSocketID() { return MAKELONG(m_wIndex,m_wRountID); }

	//��������
public:
	//��ȡ��ַ
	DWORD GetClientAddr() { return m_dwClientAddr; }
	//��ȡSOCKET����
	SOCKET GetHandle() { return m_hSocket; }
	//����ʱ��
	DWORD GetConnectTime() { return m_dwConnectTime; }
	//����ʱ��
	DWORD GetSendTickCount() { return m_dwSendTickCount; }
	//����ʱ��
	DWORD GetRecvTickCount() { return m_dwRecvTickCount; }
	//��������
	CThreadLock * GetSignedLock() { return &m_SocketLock; }
	//��׼����
	bool IsReadySend() { return m_dwRecvPacketCount>0L; }
	//�ж�����
	bool IsValidSocket() { return (m_hSocket!=INVALID_SOCKET); }

	//���ܺ���
public:
	//�󶨶���
	DWORD Attach(SOCKET hSocket, DWORD dwClientAddr);
	//���ͺ���
	bool SendData(DWORD dwCommand, void * pData, WORD wDataSize, WORD wRountID);
	//���ղ���
	bool RecvData();
	//�ر�����
	bool CloseSocket(WORD wRountID);
	//���ùر�
	bool ShutDownSocket(WORD wRountID);
	//���ñ���
	void ResetSocketData();

	//֪ͨ�ӿ�
public:
	//�������֪ͨ
	bool OnSendCompleted(COverLappedSend * pOverLappedSend, DWORD dwThancferred);
	//�������֪ͨ
	bool OnRecvCompleted(COverLappedRecv * pOverLappedRecv, DWORD dwThancferred);
	//�ر����֪ͨ
	bool OnCloseCompleted();

	//�ڲ�����
private:
	//��������
	WORD EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize);
	//��������
	WORD CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize);

	//��������
private:
	//��ȡ���ͽṹ
	inline COverLappedSend * GetSendOverLapped();

	//��������
protected:
	DWORD							m_dwClientAddr;						//���ӵ�ַ
	DWORD							m_dwConnectTime;					//����ʱ��

	//���ı���
protected:
	WORD							m_wRountID;							//ѭ������
	SOCKET							m_hSocket;							//SOCKET ���

	//״̬����
protected:
	bool							m_bNotify;							//֪ͨ��־
	bool							m_bRecvIng;							//���ձ�־
	bool							m_bCloseIng;						//�رձ�־
	WORD							m_wRecvSize;						//���ճ���
	BYTE							m_cbRecvBuf[SOCKET_BUFFER*5];		//���ջ���

	//��������
protected:
	DWORD							m_dwSendTickCount;					//����ʱ��
	DWORD							m_dwRecvTickCount;					//����ʱ��
	DWORD							m_dwSendPacketCount;				//���ͼ���
	DWORD							m_dwRecvPacketCount;				//���ܼ���

	//������
protected:
	CAuthServerCrypt				m_AuthServerCrypt;

	//�ڲ�����
protected:
	const WORD						m_wIndex;							//��������
	CThreadLock						m_SocketLock;						//ͬ������
	COverLappedRecv					m_OverLappedRecv;					//�ص��ṹ
	COverLappedSendSet				m_OverLappedSendFree;				//�ص��ṹ
	COverLappedSendSet				m_OverLappedSendActive;				//�ص��ṹ
	CServerSocketItemSink			*m_pServerSocketItemSink;			//�ص��ӿ�
};

//////////////////////////////////////////////////////////////////////////

//��д�߳���
class CServerSocketRSThread : public CServiceThread
{
	//��������
protected:
	HANDLE							m_hCompletionPort;					//��ɶ˿�

	//��������
public:
	//���캯��
	CServerSocketRSThread(void);
	//��������
	virtual ~CServerSocketRSThread(void);

	//���ܺ���
public:
	//���ú���
	bool InitThread(HANDLE hCompletionPort);

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//Ӧ���̶߳���
class CSocketAcceptThread : public CServiceThread
{
	//��������
protected:
	SOCKET							m_hListenSocket;					//��������
	HANDLE							m_hCompletionPort;					//��ɶ˿�
	CTCPSocketEngine				* m_pTCPSocketManager;				//����ָ��

	//��������
public:
	//���캯��
	CSocketAcceptThread(void);
	//��������
	virtual ~CSocketAcceptThread(void);

	//���ܺ���
public:
	//���ú���
	bool InitThread(HANDLE hCompletionPort, SOCKET hListenSocket, CTCPSocketEngine * pTCPSocketManager);

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//����߳���
class CSocketDetectThread : public CServiceThread
{
	//��������
protected:
	DWORD							m_dwTickCount;						//����ʱ��
	CTCPSocketEngine				*m_pTCPSocketManager;				//����ָ��

	//��������
public:
	//���캯��
	CSocketDetectThread(void);
	//��������
	virtual ~CSocketDetectThread(void);

	//���ܺ���
public:
	//���ú���
	bool InitThread(CTCPSocketEngine * pTCPSocketManager);

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//��˵��
typedef std::unordered_set<CServerSocketItem *> CServerSocketItemPtrSet;
typedef std::unordered_map<WORD, CServerSocketItem*> CServerSocketItemPtrMap;
typedef std::vector<CServerSocketRSThread *> CServerSocketRSThreadPtrVec;

//���������
class CTCPSocketEngine : public CQueueServiceSink, public CServerSocketItemSink
{
	friend class CServerSocketRSThread;
	friend class CSocketAcceptThread;

	//��������
public:
	//���캯��
	CTCPSocketEngine(void);
	//��������
	~CTCPSocketEngine(void);

	//����ӿ�
public:
	//��������
	bool BeginService();
	//ֹͣ����
	bool EndService();
	//���ö˿�
	bool SetServicePort(WORD wListenPort);
	//������Ŀ
	bool SetMaxSocketItem(WORD wMaxSocketItem);
	//����NODELAYģʽ
	void SetNoDelayMod(bool bMode);
	//���ýӿ�
	void SetSocketEngineSink(CQueueServiceBase * pQueueService);

	//��ѯ�ӿ�
public:
	UINT GetOnlineNum(){ return m_ActiveSocketItem.size(); }

	//���нӿ�
public:
	//֪ͨ�ص�����
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//����ӿ�
public:
	//�������
	bool DetectSocket();
	//���ͺ���
	bool SendData(DWORD dwSocketID, DWORD dwCommand, void * pData, WORD wDataSize);
	//��������
	bool SendDataBatch(DWORD dwCommand, void * pData, WORD wDataSize);
	//�ر�����
	bool CloseSocket(DWORD dwSocketID);
	//���ùر�
	bool ShutDownSocket(DWORD dwSocketID);

	//֪ͨ�ӿ�
public:
	//Ӧ����Ϣ
	virtual bool OnSocketAcceptEvent(CServerSocketItem * pServerSocketItem);
	//��ȡ��Ϣ
	virtual bool OnSocketReadEvent(DWORD dwCommand, void * pBuffer, WORD wDataSize, CServerSocketItem * pServerSocketItem);
	//�ر���Ϣ
	virtual bool OnSocketCloseEvent(CServerSocketItem * pServerSocketItem);

	//�ڲ�����
protected:
	//������ж���
	CServerSocketItem * ActiveSocketItem();
	//��ȡ����
	CServerSocketItem * EnumSocketItem(WORD wIndex);
	//�ͷ����Ӷ���
	bool FreeSocketItem(CServerSocketItem * pServerSocketItem);

	//��������
protected:
	CThreadLock						m_ItemLocked;						//����ͬ��
	CServerSocketItemPtrSet			m_FreeSocketItem;					//��������
	CServerSocketItemPtrSet			m_ActiveSocketItem;					//�����
	CServerSocketItemPtrMap			m_StorageSocketItem;				//�洢����

	//��������
protected:
	DWORD							m_dwLastDetect;						//���ʱ��
	CServerSocketItemPtrSet			m_TempSocketItem;					//��ʱ����

	//���ñ���
protected:
	WORD							m_wListenPort;						//�����˿�
	WORD							m_wMaxSocketItem;					//�������
	CQueueServiceEvent				m_AttemperEvent;					//֪ͨ���

	//�ں˱���
protected:
	bool							m_bService;							//�����־
	bool							m_bNoDelayMod;						//NoDelayģʽ
	SOCKET							m_hServerSocket;					//���Ӿ��
	HANDLE							m_hCompletionPort;					//��ɶ˿�
	CQueueService					m_SendQueueService;					//���ж���
	CSocketDetectThread				m_SocketDetectThread;				//����߳�
	CSocketAcceptThread				m_SocketAcceptThread;				//Ӧ���߳�
	CServerSocketRSThreadPtrVec		m_SocketRSThreadArray;				//��д�߳�
};

//////////////////////////////////////////////////////////////////////////