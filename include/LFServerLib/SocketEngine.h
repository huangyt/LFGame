/*
* 文件        : SocketEngine.h
* 版本        : 1.0
* 描述        : IOCP引擎接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件引用
#include "GlobalDef.h"
#include "ServiceThread.h"
#include "QueueService.h"
#include "AuthCrypt.h"

//////////////////////////////////////////////////////////////////////////
//枚举定义

//控制类型
enum enOperationType
{
	OperationType_Send,				//发送类型
	OperationType_Recv,				//接收类型
};

//////////////////////////////////////////////////////////////////////////
//类说明

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

//连接对象回调接口
class CServerSocketItemSink
{
public:
	//应答消息
	virtual bool OnSocketAcceptEvent(CServerSocketItem * pServerSocketItem)=NULL;
	//读取消息
	virtual bool OnSocketReadEvent(DWORD dwCommand, void * pBuffer, WORD wDataSize, CServerSocketItem * pServerSocketItem)=NULL;
	//关闭消息
	virtual bool OnSocketCloseEvent(CServerSocketItem * pServerSocketItem)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//重叠结构类
class COverLapped
{
	//函数定义
public:
	//构造函数
	COverLapped(enOperationType OperationType);
	//析构函数
	virtual ~COverLapped();

	//信息函数
public:
	//获取类型
	enOperationType GetOperationType() { return m_OperationType; }

	//变量定义
public:
	WSABUF							m_WSABuffer;						//数据指针
	OVERLAPPED						m_OverLapped;						//重叠结构
	const enOperationType			m_OperationType;					//操作类型
};

//接收重叠结构
class COverLappedSend : public COverLapped
{
	//函数定义
public:
	//构造函数
	COverLappedSend();
	//析构函数
	virtual ~COverLappedSend();

	//数据变量
public:
	BYTE							m_cbBuffer[SOCKET_BUFFER];			//数据缓冲
};

//重叠结构模板
template <enOperationType OperationType> class CATLOverLapped : public COverLapped
{
	//函数定义
public:
	//构造函数
	CATLOverLapped() : COverLapped(OperationType) {}
	//析构函数
	virtual ~CATLOverLapped() {}
};

//////////////////////////////////////////////////////////////////////////

//TCP SOCKET 类
class CServerSocketItem
{
	//函数定义
public:
	//构造函数
	CServerSocketItem(WORD wIndex, CServerSocketItemSink * pServerSocketItemSink);
	//析够函数
	virtual ~CServerSocketItem(void);

	//标识函数
public:
	//获取索引
	WORD GetIndex() { return m_wIndex; }
	//获取计数
	WORD GetRountID() { return m_wRountID; }
	//获取标识
	DWORD GetSocketID() { return MAKELONG(m_wIndex,m_wRountID); }

	//辅助函数
public:
	//获取地址
	DWORD GetClientAddr() { return m_dwClientAddr; }
	//获取SOCKET对象
	SOCKET GetHandle() { return m_hSocket; }
	//连接时间
	DWORD GetConnectTime() { return m_dwConnectTime; }
	//发送时间
	DWORD GetSendTickCount() { return m_dwSendTickCount; }
	//接收时间
	DWORD GetRecvTickCount() { return m_dwRecvTickCount; }
	//锁定对象
	CThreadLock * GetSignedLock() { return &m_SocketLock; }
	//是准备好
	bool IsReadySend() { return m_dwRecvPacketCount>0L; }
	//判断连接
	bool IsValidSocket() { return (m_hSocket!=INVALID_SOCKET); }

	//功能函数
public:
	//绑定对象
	DWORD Attach(SOCKET hSocket, DWORD dwClientAddr);
	//发送函数
	bool SendData(DWORD dwCommand, void * pData, WORD wDataSize, WORD wRountID);
	//接收操作
	bool RecvData();
	//关闭连接
	bool CloseSocket(WORD wRountID);
	//设置关闭
	bool ShutDownSocket(WORD wRountID);
	//重置变量
	void ResetSocketData();

	//通知接口
public:
	//发送完成通知
	bool OnSendCompleted(COverLappedSend * pOverLappedSend, DWORD dwThancferred);
	//接收完成通知
	bool OnRecvCompleted(COverLappedRecv * pOverLappedRecv, DWORD dwThancferred);
	//关闭完成通知
	bool OnCloseCompleted();

	//内部函数
private:
	//加密数据
	WORD EncryptBuffer(BYTE pcbDataBuffer[], WORD wDataSize, WORD wBufferSize);
	//解密数据
	WORD CrevasseBuffer(BYTE pcbDataBuffer[], WORD wDataSize);

	//内联函数
private:
	//获取发送结构
	inline COverLappedSend * GetSendOverLapped();

	//连接属性
protected:
	DWORD							m_dwClientAddr;						//连接地址
	DWORD							m_dwConnectTime;					//连接时间

	//核心变量
protected:
	WORD							m_wRountID;							//循环索引
	SOCKET							m_hSocket;							//SOCKET 句柄

	//状态变量
protected:
	bool							m_bNotify;							//通知标志
	bool							m_bRecvIng;							//接收标志
	bool							m_bCloseIng;						//关闭标志
	WORD							m_wRecvSize;						//接收长度
	BYTE							m_cbRecvBuf[SOCKET_BUFFER*5];		//接收缓冲

	//计数变量
protected:
	DWORD							m_dwSendTickCount;					//发送时间
	DWORD							m_dwRecvTickCount;					//接受时间
	DWORD							m_dwSendPacketCount;				//发送计数
	DWORD							m_dwRecvPacketCount;				//接受计数

	//加密类
protected:
	CAuthServerCrypt				m_AuthServerCrypt;

	//内部变量
protected:
	const WORD						m_wIndex;							//连接索引
	CThreadLock						m_SocketLock;						//同步锁定
	COverLappedRecv					m_OverLappedRecv;					//重叠结构
	COverLappedSendSet				m_OverLappedSendFree;				//重叠结构
	COverLappedSendSet				m_OverLappedSendActive;				//重叠结构
	CServerSocketItemSink			*m_pServerSocketItemSink;			//回调接口
};

//////////////////////////////////////////////////////////////////////////

//读写线程类
class CServerSocketRSThread : public CServiceThread
{
	//变量定义
protected:
	HANDLE							m_hCompletionPort;					//完成端口

	//函数定义
public:
	//构造函数
	CServerSocketRSThread(void);
	//析构函数
	virtual ~CServerSocketRSThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(HANDLE hCompletionPort);

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//应答线程对象
class CSocketAcceptThread : public CServiceThread
{
	//变量定义
protected:
	SOCKET							m_hListenSocket;					//监听连接
	HANDLE							m_hCompletionPort;					//完成端口
	CTCPSocketEngine				* m_pTCPSocketManager;				//管理指针

	//函数定义
public:
	//构造函数
	CSocketAcceptThread(void);
	//析构函数
	virtual ~CSocketAcceptThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(HANDLE hCompletionPort, SOCKET hListenSocket, CTCPSocketEngine * pTCPSocketManager);

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//检测线程类
class CSocketDetectThread : public CServiceThread
{
	//变量定义
protected:
	DWORD							m_dwTickCount;						//积累时间
	CTCPSocketEngine				*m_pTCPSocketManager;				//管理指针

	//函数定义
public:
	//构造函数
	CSocketDetectThread(void);
	//析构函数
	virtual ~CSocketDetectThread(void);

	//功能函数
public:
	//配置函数
	bool InitThread(CTCPSocketEngine * pTCPSocketManager);

	//重载函数
private:
	//运行函数
	virtual bool RepetitionRun();
};

//////////////////////////////////////////////////////////////////////////

//类说明
typedef std::unordered_set<CServerSocketItem *> CServerSocketItemPtrSet;
typedef std::unordered_map<WORD, CServerSocketItem*> CServerSocketItemPtrMap;
typedef std::vector<CServerSocketRSThread *> CServerSocketRSThreadPtrVec;

//网络管理类
class CTCPSocketEngine : public CQueueServiceSink, public CServerSocketItemSink
{
	friend class CServerSocketRSThread;
	friend class CSocketAcceptThread;

	//函数定义
public:
	//构造函数
	CTCPSocketEngine(void);
	//析构函数
	~CTCPSocketEngine(void);

	//服务接口
public:
	//启动服务
	bool BeginService();
	//停止服务
	bool EndService();
	//设置端口
	bool SetServicePort(WORD wListenPort);
	//设置数目
	bool SetMaxSocketItem(WORD wMaxSocketItem);
	//设置NODELAY模式
	void SetNoDelayMod(bool bMode);
	//设置接口
	void SetSocketEngineSink(CQueueServiceBase * pQueueService);

	//查询接口
public:
	UINT GetOnlineNum(){ return m_ActiveSocketItem.size(); }

	//队列接口
public:
	//通知回调函数
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//网络接口
public:
	//检测连接
	bool DetectSocket();
	//发送函数
	bool SendData(DWORD dwSocketID, DWORD dwCommand, void * pData, WORD wDataSize);
	//批量发送
	bool SendDataBatch(DWORD dwCommand, void * pData, WORD wDataSize);
	//关闭连接
	bool CloseSocket(DWORD dwSocketID);
	//设置关闭
	bool ShutDownSocket(DWORD dwSocketID);

	//通知接口
public:
	//应答消息
	virtual bool OnSocketAcceptEvent(CServerSocketItem * pServerSocketItem);
	//读取消息
	virtual bool OnSocketReadEvent(DWORD dwCommand, void * pBuffer, WORD wDataSize, CServerSocketItem * pServerSocketItem);
	//关闭消息
	virtual bool OnSocketCloseEvent(CServerSocketItem * pServerSocketItem);

	//内部函数
protected:
	//激活空闲对象
	CServerSocketItem * ActiveSocketItem();
	//获取对象
	CServerSocketItem * EnumSocketItem(WORD wIndex);
	//释放连接对象
	bool FreeSocketItem(CServerSocketItem * pServerSocketItem);

	//变量定义
protected:
	CThreadLock						m_ItemLocked;						//连接同步
	CServerSocketItemPtrSet			m_FreeSocketItem;					//空闲连接
	CServerSocketItemPtrSet			m_ActiveSocketItem;					//活动连接
	CServerSocketItemPtrMap			m_StorageSocketItem;				//存储连接

	//辅助变量
protected:
	DWORD							m_dwLastDetect;						//检测时间
	CServerSocketItemPtrSet			m_TempSocketItem;					//临时连接

	//配置变量
protected:
	WORD							m_wListenPort;						//监听端口
	WORD							m_wMaxSocketItem;					//最大连接
	CQueueServiceEvent				m_AttemperEvent;					//通知组件

	//内核变量
protected:
	bool							m_bService;							//服务标志
	bool							m_bNoDelayMod;						//NoDelay模式
	SOCKET							m_hServerSocket;					//连接句柄
	HANDLE							m_hCompletionPort;					//完成端口
	CQueueService					m_SendQueueService;					//队列对象
	CSocketDetectThread				m_SocketDetectThread;				//检测线程
	CSocketAcceptThread				m_SocketAcceptThread;				//应答线程
	CServerSocketRSThreadPtrVec		m_SocketRSThreadArray;				//读写线程
};

//////////////////////////////////////////////////////////////////////////