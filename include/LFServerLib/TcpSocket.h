/*
* 文件        : TcpSocket.h
* 版本        : 1.0
* 描述        : 客户端网络通信接口定义
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/26/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件引用
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "AuthCrypt.h"

//////////////////////////////////////////////////////////////////////////

//类前置声明
class CTcpSocket;
class BigNumber;

//////////////////////////////////////////////////////////////////////////

//网络钩子接口
class CTcpSocketSink
{
public:
	//网络连接消息
	virtual bool OnSocketConnect(int iErrorCode, const std::wstring& strErrorDesc, CTcpSocket * pTcpSocke)=NULL;
	//网络读取消息
	virtual bool OnSocketRead(DWORD dwCommand, void * pBuffer, WORD wDataSize, CTcpSocket * pTcpSocke)=NULL;
	//网络关闭消息
	virtual bool OnSocketClose(CTcpSocket * pTcpSocke, bool bCloseByServer)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//网络通信线程
class CSocketThread : public CServiceThread
{
	//函数定义
public:
	//构造函数
	CSocketThread(void);
	//析构函数
	virtual ~CSocketThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(CTcpSocket * pTcpSocket);

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();

	//变量定义
protected:
	CTcpSocket						*m_pTcpSocket;				//客户端连接类
};

//////////////////////////////////////////////////////////////////////////

//连接状态定义
enum enSocketState
{
	SocketState_NoConnect,			//没有连接
	SocketState_Connecting,			//正在连接
	SocketState_Connected,			//成功连接
};

//网络连接类
class CTcpSocket
{
	friend class CSocketThread;
	//函数定义
public:
	//构造函数
	CTcpSocket();
	//析构函数
	~CTcpSocket();

	//接口函数
public:
	//设置接口
	void SetSocketSink(CTcpSocketSink *pTcpSocketSink);
	//设置NODELAY模式
	bool SetNoDelayMod(bool bMode);
	//获取发送间隔
	DWORD GetLastSendTick() { return m_dwSendTickCount; }
	//获取接收间隔
	DWORD GetLastRecvTick() { return m_dwRecvTickCount; }
	//获取发送数目
	DWORD GetSendPacketCount() { return m_dwSendPacketCount; }
	//获取接收数目
	DWORD GetRecvPacketCount() { return m_dwRecvPacketCount; }
	//获取状态
	enSocketState GetConnectState() { return m_SocketState; }
	//连接服务器
	bool Connect(DWORD dwServerIP, WORD wPort);
	//连接服务器
	bool Connect(const std::wstring& strServerIP, WORD wPort);
	//发送函数
	bool SendData(DWORD dwCommand, void * pData, WORD wDataSize);
	//关闭连接
	bool CloseSocket(bool bNotify);

	//辅助函数
protected:
	//解释地址
	DWORD TranslateAddr(const std::wstring& strServerAddr);
	//连接错误
	std::wstring GetConnectError(int iErrorCode,std::wstring& strBuffer);
	//发送数据
	bool SendBuffer(void * pBuffer, WORD wSendSize);
	//加密数据
	WORD EncryptBuffer(BYTE * pcbDataBuffer, WORD wDataSize, WORD wBufferSize);
	//解密数据
	WORD CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize);

	//处理函数
protected:
	//网络连接
	LRESULT OnSocketNotifyConnect(int iErrorCode);
	//网络读取
	LRESULT OnSocketNotifyRead(int iErrorCode);
	//网络关闭
	LRESULT OnSocketNotifyClose(int iErrorCode);

	//状态变量
protected:
	bool							m_bCloseByServer;					//关闭方式
	enSocketState					m_SocketState;						//连接状态
	CTcpSocketSink					*m_pTcpSocketSink;					//回调接口

	//核心变量
protected:
	SOCKET							m_hSocket;							//连接句柄
	HANDLE							m_hSocketEvent;						//连接事件
	WORD							m_wRecvSize;						//接收长度
	BYTE							m_cbRecvBuf[SOCKET_BUFFER*10];		//接收缓冲
	CSocketThread					m_SocketThread;						//事件线程

	//计数变量
protected:
	DWORD							m_dwSendTickCount;					//发送时间
	DWORD							m_dwRecvTickCount;					//接收时间
	DWORD							m_dwSendPacketCount;				//发送计数
	DWORD							m_dwRecvPacketCount;				//接受计数

	//加密类
protected:
	CAuthClientCrypt				m_AuthClientCrypt;
};

//////////////////////////////////////////////////////////////////////////