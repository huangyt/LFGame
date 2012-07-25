#include "AsynchronismEngine.h"

#include <iostream>

//////////////////////////////////////////////////////////////////////////

//构造函数
CMessageThread::CMessageThread()
{
	m_pAsynchronismEngine=NULL;
	m_bRunning = true;
	m_hMsgEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//析构函数
CMessageThread::~CMessageThread()
{
	for(UINT i=0; i<m_MsgQueue.size(); i++)
	{
		SAFE_DELETE(m_MsgQueue.front());
		m_MsgQueue.pop();
	}
	CloseHandle(m_hMsgEvent);
}

//运行函数
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
	//创建消息
	stMsg *pMsg = new stMsg;
	pMsg->_wParam = wParam;
	pMsg->_lParam = lParam;

	CThreadLockHandle ThreadLockHandle(&m_MsgLock);
	m_MsgQueue.push(pMsg);
	SetEvent(m_hMsgEvent);
}

//停止线程
bool CMessageThread::StopThread(DWORD dwWaitSeconds)
{
	m_bRunning = false;
	SetEvent(m_hMsgEvent);

	return CServiceThread::StopThread(dwWaitSeconds);
}

//开始事件
bool CMessageThread::OnThreadStratEvent()
{
	ASSERT(m_pAsynchronismEngine!=NULL);
	return m_pAsynchronismEngine->OnMessageThreadStart();
}

//停止事件
bool CMessageThread::OnThreadStopEvent()
{
	ASSERT(m_pAsynchronismEngine!=NULL);
	return m_pAsynchronismEngine->OnMessageThreadStop();
}

//请求消息
bool CMessageThread::OnAsynRequest(WPARAM wParam, LPARAM lParam)
{
	//效验变量
	ASSERT(m_pAsynchronismEngine!=NULL);

	//获取参数
	WORD wRequestID=(WORD)wParam;
	CAsynchronismEngineSink * pAsynchronismEngineSink=(CAsynchronismEngineSink *)lParam;

	//请求处理
	ASSERT(pAsynchronismEngineSink!=NULL);
	m_pAsynchronismEngine->OnAsynchronismRequest(wRequestID,pAsynchronismEngineSink);

	return true;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CAsynchronismEngine::CAsynchronismEngine(void)
{
	//设置变量
	m_bService=false;
	memset(m_cbBuffer,0,sizeof(m_cbBuffer));

	//设置组件
	m_MessageThread.m_pAsynchronismEngine=this;

	return;
}

//析构函数
CAsynchronismEngine::~CAsynchronismEngine(void)
{
}

//启动服务
bool CAsynchronismEngine::BeginService()
{
	//启动线程
	if (m_MessageThread.StartThead()==false) return false;

	//设置变量
	m_bService=true;

	return true;
}

//停止服务
bool CAsynchronismEngine::EndService()
{
	//设置变量
	m_bService=false;

	//停止线程
	m_MessageThread.StopThread();

	//清理数据
	m_DataStorage.RemoveData(false);

	return true;
}

//插入请求
bool CAsynchronismEngine::InsertRequest(WORD wRequestID, void * const pBuffer, WORD wDataSize, CAsynchronismEngineSink *pAsynchronismEngineSink)
{
	//效验状态
	ASSERT(wDataSize<=sizeof(m_cbBuffer));
	if (m_bService==false) return false;
	if (wDataSize>sizeof(m_cbBuffer)) return false;

	//查询接口
	ASSERT(pAsynchronismEngineSink!=NULL);
	if (pAsynchronismEngineSink==NULL) return false;

	//插入数据
	CThreadLockHandle ThreadLockHandle(&m_ThreadLock);
	if (m_DataStorage.AddData(wRequestID,pBuffer,wDataSize)==false) return false;

	//投递消息
	m_MessageThread.PostMessage(wRequestID,(LPARAM)pAsynchronismEngineSink);

	return true;
}

//注册钩子
bool CAsynchronismEngine::RegisterAsynchronismEngineSink(CAsynchronismEngineSink *pAsynchronismEngineSink)
{
	ASSERT(pAsynchronismEngineSink!=NULL);
	if (pAsynchronismEngineSink==NULL) return false;

	//查找现存
	CAsynchronismEngineSinkSet::iterator iter = m_AsynchronismEngineSinkSet.find(pAsynchronismEngineSink);
	if(iter != m_AsynchronismEngineSinkSet.end()) return false;
 
 	//注册钩子
 	m_AsynchronismEngineSinkSet.insert(pAsynchronismEngineSink);

	return true;
}

//取消注册
bool CAsynchronismEngine::UnRegisterAsynchronismEngineSink(CAsynchronismEngineSink *pAsynchronismEngineSink)
{
	ASSERT(pAsynchronismEngineSink!=NULL);
	if (pAsynchronismEngineSink==NULL) return false;

	//删除钩子
	CAsynchronismEngineSinkSet::iterator iter = m_AsynchronismEngineSinkSet.find(pAsynchronismEngineSink);
	if(iter != m_AsynchronismEngineSinkSet.end())
	{
		m_AsynchronismEngineSinkSet.erase(iter);
		return true;
	}

	return false;
}

//线程启动
bool CAsynchronismEngine::OnMessageThreadStart()
{
	//事件通知
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

//线程停止
bool CAsynchronismEngine::OnMessageThreadStop()
{
	//事件通知
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

//异步请求
void CAsynchronismEngine::OnAsynchronismRequest(WORD wRequestID, CAsynchronismEngineSink * pAsynchronismEngineSink)
{
	//锁定资源
	CThreadLockHandle ThreadLockHandle(&m_ThreadLock);

	//提取数据
	tagDataHead DataHead;
	m_DataStorage.GetData(DataHead,m_cbBuffer,sizeof(m_cbBuffer));

	//消息处理
	try
	{
		ASSERT(DataHead.wIdentifier==wRequestID);
		pAsynchronismEngineSink->OnAsynchronismRequest(wRequestID,m_cbBuffer,DataHead.wDataSize);
	}
	catch (...) {}

	return;
}

//////////////////////////////////////////////////////////////////////////