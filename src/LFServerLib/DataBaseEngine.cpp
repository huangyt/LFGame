#include "DataBaseEngine.h"
#include "FormatString.h"

//////////////////////////////////////////////////////////////////////////

//效验结果宏
#define EfficacyResult(hResult) { if (FAILED(hResult)) _com_issue_error(hResult); }

//////////////////////////////////////////////////////////////////////////

//构造函数
CADOError::CADOError()
{
	m_enErrorType=ErrorType_Nothing;
}

//析构函数
CADOError::~CADOError()
{
}

//设置错误
void CADOError::SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe)
{
	//设置错误
	m_enErrorType=enErrorType;
	m_strErrorDescribe=std::move(strDescribe);

	//抛出错误
	throw this;

	return;
}

//////////////////////////////////////////////////////////////////////////

//数据集对象
CQueryResult::CQueryResult(CObjectFactory<CQueryResult>* pFactory)
	:m_pObjectFactory(pFactory)
{
	//创建对象
	m_DBRecordset.CreateInstance(__uuidof(Recordset));

	//效验数据
	ASSERT(m_DBRecordset!=NULL);
	if (m_DBRecordset==NULL) throw TEXT("数据库记录集对象创建失败");
}

CQueryResult::~CQueryResult()
{
	CloseRecordset();
	//释放对象
	m_DBRecordset.Release();
}

void CQueryResult::Release()
{
	CloseRecordset();

	m_pObjectFactory->ReleaseObject(this);
}

void CQueryResult::Attach(const _RecordsetPtr &ptr)
{
	m_DBRecordset = ptr;
}

//是否打开
bool CQueryResult::IsRecordsetOpened()
{
	if (m_DBRecordset==NULL) return false;
	if (m_DBRecordset->GetState()==adStateClosed) return false;
	return true;
}

//关闭记录
bool CQueryResult::CloseRecordset()
{
	try 
	{
		if (IsRecordsetOpened()) EfficacyResult(m_DBRecordset->Close());
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取错误
std::string CQueryResult::GetComErrorDescribe(CComError & ComError)
{
	_bstr_t bstrDescribe(ComError.Description());
	m_strErrorDescribe = std::move(strFormat("ADO 错误：0x%8x，%s",ComError.Error(),(char*)bstrDescribe));
	return m_strErrorDescribe;
}

//设置错误
void CQueryResult::SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe)
{
	m_ADOError.SetErrorInfo(enErrorType,strDescribe);
	return;
}

//往下移动
void CQueryResult::MoveToNext()
{
	try 
	{ 
		m_DBRecordset->MoveNext(); 
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return;
}

//移到开头
void CQueryResult::MoveToFirst()
{
	try 
	{ 
		m_DBRecordset->MoveFirst(); 
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return;
}

//是否结束
bool CQueryResult::IsEndRecordset()
{
	try 
	{
		return (m_DBRecordset->adoEOF==VARIANT_TRUE); 
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return true;
}

//获取数目
long CQueryResult::GetRecordCount()
{
	try
	{
		if (m_DBRecordset==NULL) return 0;
		return m_DBRecordset->GetRecordCount();
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return 0;
}

//获取大小
long CQueryResult::GetActualSize(const std::string& strParamName)
{
	try 
	{
		return m_DBRecordset->Fields->Item[strParamName.c_str()]->ActualSize;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return -1;
}

//获取数据
void CQueryResult::GetRecordsetValue(const std::string& strFieldName, CDBVarValue & DBVarValue)
{
	try
	{
		DBVarValue = m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, BYTE & bValue)
{
	try
	{
		bValue=0;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		switch(vtFld.vt)
		{
		case VT_BOOL:
			{
				bValue=(vtFld.boolVal!=0)?1:0;
				break;
			}
		case VT_I2:
		case VT_UI1:
			{
				bValue=(vtFld.iVal>0)?1:0;
				break;
			}
		case VT_NULL:
		case VT_EMPTY:
			{
				bValue=0;
				break;
			}
		default: bValue=(BYTE)vtFld.iVal;
		}	
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, WORD & wValue)
{
	try
	{
		wValue=0L;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		if ((vtFld.vt!=VT_NULL)&&(vtFld.vt!=VT_EMPTY)) wValue=(WORD)vtFld.ulVal;
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, INT & nValue)
{
	try
	{
		nValue=0;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		switch(vtFld.vt)
		{
		case VT_BOOL:
			{
				nValue = vtFld.boolVal;
				break;
			}
		case VT_I2:
		case VT_UI1:
			{
				nValue = vtFld.iVal;
				break;
			}
		case VT_NULL:
		case VT_EMPTY:
			{
				nValue = 0;
				break;
			}
		default: nValue = vtFld.iVal;
		}	
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, LONG & lValue)
{
	try
	{
		lValue=0L;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		if ((vtFld.vt!=VT_NULL)&&(vtFld.vt!=VT_EMPTY)) lValue=vtFld.lVal;
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, DWORD & ulValue)
{
	try
	{
		ulValue=0L;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		if ((vtFld.vt!=VT_NULL)&&(vtFld.vt!=VT_EMPTY)) ulValue=vtFld.ulVal;
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, UINT & ulValue)
{
	try
	{
		ulValue=0L;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		if ((vtFld.vt!=VT_NULL)&&(vtFld.vt!=VT_EMPTY)) ulValue=vtFld.lVal;
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, DOUBLE & dbValue)
{
	try
	{
		dbValue=0.0L;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		switch(vtFld.vt)
		{
		case VT_R4:
			{
				dbValue=vtFld.fltVal;
				break;
			}
		case VT_R8:
			{
				dbValue=vtFld.dblVal;
				break;
			}
		case VT_DECIMAL:
			{
				dbValue=vtFld.decVal.Lo32;
				dbValue*=(vtFld.decVal.sign==128)?-1:1;
				dbValue/=pow((float)10,vtFld.decVal.scale); 
				break;
			}
		case VT_UI1:
			{
				dbValue=vtFld.iVal;
				break;
			}
		case VT_I2:
		case VT_I4:
			{
				dbValue=vtFld.lVal;
				break;
			}
		case VT_NULL:
		case VT_EMPTY:
			{
				dbValue=0.0L;
				break;
			}
		default: dbValue=vtFld.dblVal;
		}
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, __int64 & llValue)
{
	try
	{
		llValue=0L;
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		if ((vtFld.vt!=VT_NULL)&&(vtFld.vt!=VT_EMPTY))
		{
			vtFld.ChangeType(VT_I8);
			llValue=(__int64)vtFld.decVal.Lo64;
			return true;
		}
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, std::string & strValue)
{
	try
	{
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		if (vtFld.vt==VT_BSTR || vtFld.vt==VT_DATE)
		{
			strValue = (char*)_bstr_t(vtFld);
			return true;
		}
		return false;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, CDateTime & Time)
{
	try
	{
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		switch(vtFld.vt) 
		{
		case VT_DATE:
			{
				Time.SetTime(vtFld);
				break;
			}
		case VT_EMPTY:
		case VT_NULL:
			{
				Time.SetNULL();
				break;
			}
		default: return false;
		}
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//获取参数
bool CQueryResult::GetFieldValue(const std::string& strFieldName, bool & bValue)
{
	try
	{
		CDBVarValue vtFld=m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
		switch(vtFld.vt) 
		{
		case VT_BOOL:
			{
				bValue=(vtFld.boolVal==0)?false:true;
				break;
			}
		case VT_EMPTY:
		case VT_NULL:
			{
				bValue = false;
				break;
			}
		default:return false;
		}
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//////////////////////////////////////////////////////////////////////////

CObjectFactory<CQueryResult> CDataBase::m_QueryResultFactory;

//构造函数
CDataBase::CDataBase() : m_dwResumeConnectCount(30L),m_dwResumeConnectTime(30L)
{
	//状态变量
	m_dwConnectCount=0;
	m_dwConnectErrorTime=0L;

	//创建对象
	m_DBConnection.CreateInstance(__uuidof(Connection));

	//效验数据
	ASSERT(m_DBConnection!=NULL);
	if (m_DBConnection==NULL) throw TEXT("数据库连接对象创建失败");
}

//析构函数
CDataBase::~CDataBase()
{
	//关闭连接
	CloseConnection();

	//释放对象
	m_DBConnection.Release();
	return;
}

//打开连接
bool CDataBase::OpenConnection()
{
	//连接数据库
	try
	{
		//关闭连接
		CloseConnection();

		//连接数据库
		EfficacyResult(m_DBConnection->Open(_bstr_t(m_strConnect.c_str()),L"",L"",adConnectUnspecified));
		m_DBConnection->CursorLocation=adUseClient;

		//设置变量
		m_dwConnectCount=0L;
		m_dwConnectErrorTime=0L;

		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//关闭连接
bool CDataBase::CloseConnection()
{
	try
	{
		if ((m_DBConnection!=NULL)&&(m_DBConnection->GetState()!=adStateClosed))
		{
			EfficacyResult(m_DBConnection->Close());
		}
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//重新连接
bool CDataBase::TryConnectAgain(bool bFocusConnect, CComError * pComError)
{
	try
	{
		//判断重连
		bool bReConnect=bFocusConnect;
		if (bReConnect==false)
		{
			DWORD dwNowTime=(DWORD)time(NULL);
			if ((m_dwConnectErrorTime+m_dwResumeConnectTime)>dwNowTime) bReConnect=true;
		}
		if ((bReConnect==false)&&(m_dwConnectCount>m_dwResumeConnectCount)) bReConnect=true;

		//设置变量
		m_dwConnectCount++;
		m_dwConnectErrorTime=(DWORD)time(NULL);
		if (bReConnect==false)
		{
			if (pComError!=NULL) SetErrorInfo(ErrorType_Connect,GetComErrorDescribe(*pComError));
			return false;
		}

		//重新连接
		OpenConnection();
		return true;
	}
	catch (CComError & ComError)
	{
		//重新连接错误
		SetErrorInfo(ErrorType_Connect,GetComErrorDescribe(ComError));
	}

	return false;
}

void CDataBase::SetConnectionInfo(const std::string& strDBPath, const std::string& strPass)
{	
	m_strConnect = std::move(strFormat("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;Jet OLEDB:Database Password=%s;",strDBPath.c_str(),strPass.c_str()));
}

//是否连接错误
bool CDataBase::IsConnectError()
{
	try 
	{
		//状态判断
		if (m_DBConnection==NULL) return true;
		if (m_DBConnection->GetState()==adStateClosed) return true;

		//参数判断
		long lErrorCount=m_DBConnection->Errors->Count;
		if (lErrorCount>0L)
		{
			ErrorPtr pError=NULL;
			for(long i=0;i<lErrorCount;i++)
			{
				pError=m_DBConnection->Errors->GetItem(i);
				if (pError->Number==0x80004005) return true;
			}
		}

		return false;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//转义特殊字符，防止SQL注入
void CDataBase::EscapeString(std::string& str)
{
	std::string temp;
	for(std::string::size_type n = 0; n < str.size(); ++n)
	{
		switch(str[n])
		{
		case '\'':
			{
				temp += "\\\'";
				break;
			}
		case '"':
			{
				temp += "\\\"";
				break;
			}
		case '\\':
			{
				temp += "\\\\";
				break;
			}
		default:
			temp += str[n];
			break;
		}
	}
	str = temp;
}

//执行语句
bool CDataBase::Execute(const std::string& strSql)
{
	CThreadLockHandle Lock(&m_DBLock);
	try
	{
		m_DBConnection->CursorLocation=adUseClient;
		m_DBConnection->Execute(strSql.c_str(),NULL,adCmdText);
		return true;
	}
	catch (CComError & ComError) 
	{ 
		if (IsConnectError()==true)	TryConnectAgain(false,&ComError);
		else SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError));
	}

	return false;
}

//查询语句
CQueryResult* CDataBase::Query(const std::string& strSql)
{
	CThreadLockHandle Lock(&m_DBLock);
	try
	{
		m_DBConnection->CursorLocation=adUseClient;
		CQueryResult *pResult = m_QueryResultFactory.GetFreeObject();
		pResult->Attach(m_DBConnection->Execute(strSql.c_str(),NULL,adCmdText));
		m_QueryResultFactory.InsertActiveObject(pResult);
		return pResult;
	}
	catch (CComError & ComError) 
	{ 
		if (IsConnectError()==true)	TryConnectAgain(false,&ComError);
		else SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError));
	}

	return NULL;
}

//开始事务
bool CDataBase::BeginTransaction(void)
{
	try
	{
		m_DBConnection->BeginTrans();
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }
	return false;
}

//提交事务
bool CDataBase::CommitTransaction(void)
{
	try
	{
		m_DBConnection->CommitTrans();
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }
	return false;
}

//回滚事务
bool CDataBase::RollbackTransaction(void)
{
	try
	{
		m_DBConnection->RollbackTrans();
		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }
	return false;
}

//获取错误
std::string CDataBase::GetComErrorDescribe(CComError & ComError)
{
	_bstr_t bstrDescribe(ComError.Description());
	m_strErrorDescribe = std::move(strFormat("ADO 错误：0x%8x，%s",ComError.Error(),(char*)bstrDescribe));
	return m_strErrorDescribe;
}

//设置错误
void CDataBase::SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe)
{
	m_ADOError.SetErrorInfo(enErrorType,strDescribe);
	return;
}

//////////////////////////////////////////////////////////////////////////

//构造函数
CDataBaseEngine::CDataBaseEngine(void)
{
	//设置变量
	m_bService=false;
	m_pDataBaseSink=NULL;

	//初始化 COM
	CoInitialize(NULL);

	return;
}

//析构函数
CDataBaseEngine::~CDataBaseEngine(void)
{
	//卸载 COM
	CoUninitialize();
}

//注册接口
void CDataBaseEngine::SetDataBaseSink(CDataBaseSink *pDataBaseSink)
{
	//效验参数
	ASSERT(pDataBaseSink!=NULL);

	//设置接口
	m_pDataBaseSink = pDataBaseSink;
}

//获取接口
CQueueService* CDataBaseEngine::GetQueueService()
{
	return &m_RequestQueueService;
}

//启动服务
bool CDataBaseEngine::BeginService()
{
	//判断状态
	if (m_bService==true) 
	{
		return true;
	}

	//外挂接口
	if (m_pDataBaseSink==NULL)
	{
		return false;
	}

	//设置队列
	m_RequestQueueService.SetQueueServiceSink(static_cast<CQueueServiceSink*>(this));

	//启动外挂
	if (m_pDataBaseSink->BeginService()==false)
	{
		return false;
	}

	//启动队列
	if (m_RequestQueueService.BeginService()==false)
	{
		return false;
	}

	//设置变量
	m_bService=true;

	return true;
}

//停止服务
bool CDataBaseEngine::EndService()
{
	//设置变量
	m_bService=false;

	//停止请求队列
	m_RequestQueueService.EndService();

	//停止外挂
	if (m_pDataBaseSink!=NULL)
	{
		m_pDataBaseSink->EndService();
	}

	return true;
}

//队列接口
void CDataBaseEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	//判断状态
	if (m_bService==false) return;

	//请求处理
	switch (wIdentifier)
	{
	case EVENT_DATABASE:
		{
			//效验参数
			ASSERT(pBuffer!=NULL);
			ASSERT(wDataSize>=sizeof(NTY_DataBaseEvent));
			if (wDataSize<sizeof(NTY_DataBaseEvent)) return;

			//变量定义
			NTY_DataBaseEvent * pDataBaseEvent=(NTY_DataBaseEvent *)pBuffer;
			pDataBaseEvent->pDataBuffer=pDataBaseEvent+1;

			//处理数据
			ASSERT(m_pDataBaseSink!=NULL);
			m_pDataBaseSink->OnDataBaseRequest(*pDataBaseEvent);

			return;
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////