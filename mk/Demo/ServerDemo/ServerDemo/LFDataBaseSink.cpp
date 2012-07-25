#include "LFCommon.h"
#include "LFDataBaseSink.h"


CLFDataBaseSink::CLFDataBaseSink(void)
{
}

CLFDataBaseSink::~CLFDataBaseSink(void)
{
}

//��ʼ��
void CLFDataBaseSink::InitDataBaseSink(CQueueService *pAttemperServer)
{
	m_AttemperEvent.SetQueueService(pAttemperServer);
}

CDataBase* CLFDataBaseSink::GetLoginDataBase()
{
	return &m_LoginDataBase;
}

//���ݿ�ģ������
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

	sLogEngine.Log(Level_Normal, "���ݿ���������...");

	return true;
}

//���ݿ�ģ��ر�
bool CLFDataBaseSink::EndService()
{
	m_LoginDataBase.CloseConnection();

	sLogEngine.Log(Level_Normal, "���ݿ�����ر�...");
	return true;
}

//���ݲ�������
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
// 		sLogEngine.Log(Level_Normal, "��Ϸ���룺%s", strPassword.c_str());
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