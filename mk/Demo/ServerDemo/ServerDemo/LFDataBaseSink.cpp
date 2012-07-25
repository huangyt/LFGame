#include "LFCommon.h"
#include "LFDataBaseSink.h"


CLFDataBaseSink::CLFDataBaseSink(void)
{
}

CLFDataBaseSink::~CLFDataBaseSink(void)
{
}

//初始化
void CLFDataBaseSink::InitDataBaseSink(CQueueService *pAttemperServer)
{
	m_AttemperEvent.SetQueueService(pAttemperServer);
}

CDataBase* CLFDataBaseSink::GetLoginDataBase()
{
	return &m_LoginDataBase;
}

//数据库模块启动
bool CLFDataBaseSink::BeginService()
{
	m_LoginDataBase.SetConnectionInfo("LoginDB.mdb", "lifeng");
	try
	{
		m_LoginDataBase.OpenConnection();
	}
	catch(CADOError *pADOError)
	{
		MessageBoxA(NULL, pADOError->GetErrorDescribe().c_str(),"error",MB_OK);
		return false;
	}

	sLogEngine.Log(Level_Normal, "数据库引擎启动...");

	return true;
}

//数据库模块关闭
bool CLFDataBaseSink::EndService()
{
	m_LoginDataBase.CloseConnection();

	sLogEngine.Log(Level_Normal, "数据库引擎关闭...");
	return true;
}

//数据操作处理
bool CLFDataBaseSink::OnDataBaseRequest(const NTY_DataBaseEvent & DataBaseEvent)
{
//  	try
//  	{
// 		std::string strSql = "select * from UserInfo where Account = \'lifeng\'";
// 		CQueryResult *pResult =  m_UserDataBase.Query(strSql);
// 		std::string strPassword;
// 		if(!pResult->IsEndRecordset())
// 		{
// 			pResult->GetFieldValue("Password", strPassword);
// 		}
// 		pResult->Release();
// 		sLogEngine.Log(Level_Normal, "游戏密码：%s", strPassword.c_str());
// 	}
// 	catch(CADOError *err)
// 	{
// 		sLogEngine.Log(Level_Normal, err->GetErrorDescribe());
// 	}
// 
// 	m_AttemperEvent.PostDataBaseEvent(DataBaseEvent.wRequestID, 
// 		DataBaseEvent.dwSessionID, NULL, 0);
	return true;
}