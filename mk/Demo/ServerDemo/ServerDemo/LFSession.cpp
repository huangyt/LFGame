#include "LFCommon.h"
#include "LFSession.h"
#include "CMDDefine.h"

#define RegisterCMD(CMD, Handle) m_CommandMap.insert(std::make_pair(##CMD, &CLFSession::##Handle))

CLFSession::CLFSession(CSessionFactory* pFactory)
	:m_pSessionFactory(pFactory)
	,m_dwSocketID(0)
	,m_dwClientIP(0)
{
	RegisterCMD(CMD_LF_LOGON_CHALLENGE, HandleLogonChallenge);
}

void CLFSession::Initialize(DWORD dwSocketID, DWORD dwClientIP)
{
	m_dwSocketID = dwSocketID;
	m_dwClientIP = dwClientIP;
}

void CLFSession::Uninitialize()
{
}

bool CLFSession::SendData(DWORD dwCommand, void * pData, WORD wDataSize)
{
	return sGameServer.GetTCPSocketEngine()->SendData(m_dwSocketID, dwCommand, pData, wDataSize);
}

void CLFSession::ReceiveTimerEvent(WORD wTimerID, WPARAM wBindParam)
{
}

void CLFSession::ReceiveDataBaseEvent(WORD wRequestID, WORD wDataSize, void* pDataBuffer)
{
}

bool CLFSession::ReceiveSocketReadEvent(DWORD dwCommand, WORD wDataSize, void* pDataBuffer)
{
	sLogEngine.Log(Level_Normal, "收到消息 0x%08x", dwCommand);

	m_RecvBuffer.resize(wDataSize);
	if(NULL != pDataBuffer && 0 != wDataSize)
	{
		m_RecvBuffer.put(0, static_cast<BYTE*>(pDataBuffer), wDataSize);
	}

	CCommandMap::iterator iter = m_CommandMap.find(dwCommand);
	if(iter != m_CommandMap.end())
	{
		return (this->*iter->second)();
	}

	sLogEngine.Log(Level_Exception, "未知消息 0x%08x", dwCommand);

	return false;
}

bool CLFSession::HandleLogonChallenge()
{
	if(m_RecvBuffer.size() < sizeof(sLFLogonChanllenge))
		return false;

	m_SendBuffer.clear();
	
	sLFLogonChanllenge *cl = (sLFLogonChanllenge*)m_RecvBuffer.contents();
	std::string strUserName = (char*)cl->I;
	sGameServer.GetLoginDatabase()->EscapeString(strUserName);

	sLogEngine.Log(Level_Normal, "客户端尝试登录...");
	sLogEngine.Log(Level_Normal, "客户端版本: %d.%d.%d.%d 用户名: %s", cl->version1, cl->version2, cl->version3, cl->build, strUserName.c_str());

 	try
 	{
		std::string strSql = strFormat("select * from UserInfo where Account = \'%s\'", strUserName.c_str());
		CQueryResult *pResult =  sGameServer.GetLoginDatabase()->Query(strSql);
		std::string strPassword;
		UINT nID = 0;
		if(!pResult->IsEndRecordset())
		{
			pResult->GetFieldValue("Password", strPassword);
			pResult->GetFieldValue("ID", nID);

			sLogEngine.Log(Level_Normal, "ID = %d, 游戏密码：%s", nID, strPassword.c_str());

			m_SendBuffer<<(DWORD)RET_SUCCESS;

			SendData(CMD_LF_LOGON_CHALLENGE, (void*)m_SendBuffer.contents(), m_SendBuffer.size());
		}
		else
		{
			sLogEngine.Log(Level_Normal, "%s 不存在", strUserName.c_str());

			m_SendBuffer<<(DWORD)RET_FAIL_UNKNOWN_ACCOUNT;

			SendData(CMD_LF_LOGON_CHALLENGE, (void*)m_SendBuffer.contents(), m_SendBuffer.size());
		}
		pResult->Release();
	}
	catch(CADOError *err)
	{
		sLogEngine.Log(Level_Exception, err->GetErrorDescribe());
	}

	return true;
}