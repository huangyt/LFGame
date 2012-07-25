/*
* �ļ�        : TcpSocket.h
* �汾        : 1.0
* ����        : �ͻ�������ͨ�Žӿڶ���
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/26/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "AuthCrypt.h"

//////////////////////////////////////////////////////////////////////////

//��ǰ������
class CTcpSocket;
class BigNumber;

//////////////////////////////////////////////////////////////////////////

//���繳�ӽӿ�
class CTcpSocketSink
{
public:
	//����������Ϣ
	virtual bool OnSocketConnect(int iErrorCode, const std::wstring& strErrorDesc, CTcpSocket * pTcpSocke)=NULL;
	//�����ȡ��Ϣ
	virtual bool OnSocketRead(DWORD dwCommand, void * pBuffer, WORD wDataSize, CTcpSocket * pTcpSocke)=NULL;
	//����ر���Ϣ
	virtual bool OnSocketClose(CTcpSocket * pTcpSocke, bool bCloseByServer)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//����ͨ���߳�
class CSocketThread : public CServiceThread
{
	//��������
public:
	//���캯��
	CSocketThread(void);
	//��������
	virtual ~CSocketThread(void);

	//���ܺ���
public:
	//���ú���
	bool InitThread(CTcpSocket * pTcpSocket);

	//���غ���
private:
	//���к���
	virtual bool RepetitionRun();

	//��������
protected:
	CTcpSocket						*m_pTcpSocket;				//�ͻ���������
};

//////////////////////////////////////////////////////////////////////////

//����״̬����
enum enSocketState
{
	SocketState_NoConnect,			//û������
	SocketState_Connecting,			//��������
	SocketState_Connected,			//�ɹ�����
};

//����������
class CTcpSocket
{
	friend class CSocketThread;
	//��������
public:
	//���캯��
	CTcpSocket();
	//��������
	~CTcpSocket();

	//�ӿں���
public:
	//���ýӿ�
	void SetSocketSink(CTcpSocketSink *pTcpSocketSink);
	//����NODELAYģʽ
	bool SetNoDelayMod(bool bMode);
	//��ȡ���ͼ��
	DWORD GetLastSendTick() { return m_dwSendTickCount; }
	//��ȡ���ռ��
	DWORD GetLastRecvTick() { return m_dwRecvTickCount; }
	//��ȡ������Ŀ
	DWORD GetSendPacketCount() { return m_dwSendPacketCount; }
	//��ȡ������Ŀ
	DWORD GetRecvPacketCount() { return m_dwRecvPacketCount; }
	//��ȡ״̬
	enSocketState GetConnectState() { return m_SocketState; }
	//���ӷ�����
	bool Connect(DWORD dwServerIP, WORD wPort);
	//���ӷ�����
	bool Connect(const std::wstring& strServerIP, WORD wPort);
	//���ͺ���
	bool SendData(DWORD dwCommand, void * pData, WORD wDataSize);
	//�ر�����
	bool CloseSocket(bool bNotify);

	//��������
protected:
	//���͵�ַ
	DWORD TranslateAddr(const std::wstring& strServerAddr);
	//���Ӵ���
	std::wstring GetConnectError(int iErrorCode,std::wstring& strBuffer);
	//��������
	bool SendBuffer(void * pBuffer, WORD wSendSize);
	//��������
	WORD EncryptBuffer(BYTE * pcbDataBuffer, WORD wDataSize, WORD wBufferSize);
	//��������
	WORD CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize);

	//������
protected:
	//��������
	LRESULT OnSocketNotifyConnect(int iErrorCode);
	//�����ȡ
	LRESULT OnSocketNotifyRead(int iErrorCode);
	//����ر�
	LRESULT OnSocketNotifyClose(int iErrorCode);

	//״̬����
protected:
	bool							m_bCloseByServer;					//�رշ�ʽ
	enSocketState					m_SocketState;						//����״̬
	CTcpSocketSink					*m_pTcpSocketSink;					//�ص��ӿ�

	//���ı���
protected:
	SOCKET							m_hSocket;							//���Ӿ��
	HANDLE							m_hSocketEvent;						//�����¼�
	WORD							m_wRecvSize;						//���ճ���
	BYTE							m_cbRecvBuf[SOCKET_BUFFER*10];		//���ջ���
	CSocketThread					m_SocketThread;						//�¼��߳�

	//��������
protected:
	DWORD							m_dwSendTickCount;					//����ʱ��
	DWORD							m_dwRecvTickCount;					//����ʱ��
	DWORD							m_dwSendPacketCount;				//���ͼ���
	DWORD							m_dwRecvPacketCount;				//���ܼ���

	//������
protected:
	CAuthClientCrypt				m_AuthClientCrypt;
};

//////////////////////////////////////////////////////////////////////////