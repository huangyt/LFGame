#include "DataBaseEngine.h"
#include "FormatString.h"

//////////////////////////////////////////////////////////////////////////

//Ч������
#define EfficacyResult(hResult) { if (FAILED(hResult)) _com_issue_error(hResult); }

//////////////////////////////////////////////////////////////////////////

//���캯��
CADOError::CADOError()
{
	m_enErrorType=ErrorType_Nothing;
}

//��������
CADOError::~CADOError()
{
}

//���ô���
void CADOError::SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe)
{
	//���ô���
	m_enErrorType=enErrorType;
	m_strErrorDescribe=std::move(strDescribe);

	//�׳�����
	throw this;

	return;
}

//////////////////////////////////////////////////////////////////////////

//���ݼ�����
CQueryResult::CQueryResult(CObjectFactory<CQueryResult>* pFactory)
	:m_pObjectFactory(pFactory)
{
	//��������
	m_DBRecordset.CreateInstance(__uuidof(Recordset));

	//Ч������
	ASSERT(m_DBRecordset!=NULL);
	if (m_DBRecordset==NULL) throw TEXT("���ݿ��¼�����󴴽�ʧ��");
}

CQueryResult::~CQueryResult()
{
	CloseRecordset();
	//�ͷŶ���
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

//�Ƿ��
bool CQueryResult::IsRecordsetOpened()
{
	if (m_DBRecordset==NULL) return false;
	if (m_DBRecordset->GetState()==adStateClosed) return false;
	return true;
}

//�رռ�¼
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

//��ȡ����
std::string CQueryResult::GetComErrorDescribe(CComError & ComError)
{
	_bstr_t bstrDescribe(ComError.Description());
	m_strErrorDescribe = std::move(strFormat("ADO ����0x%8x��%s",ComError.Error(),(char*)bstrDescribe));
	return m_strErrorDescribe;
}

//���ô���
void CQueryResult::SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe)
{
	m_ADOError.SetErrorInfo(enErrorType,strDescribe);
	return;
}

//�����ƶ�
void CQueryResult::MoveToNext()
{
	try 
	{ 
		m_DBRecordset->MoveNext(); 
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return;
}

//�Ƶ���ͷ
void CQueryResult::MoveToFirst()
{
	try 
	{ 
		m_DBRecordset->MoveFirst(); 
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return;
}

//�Ƿ����
bool CQueryResult::IsEndRecordset()
{
	try 
	{
		return (m_DBRecordset->adoEOF==VARIANT_TRUE); 
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return true;
}

//��ȡ��Ŀ
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

//��ȡ��С
long CQueryResult::GetActualSize(const std::string& strParamName)
{
	try 
	{
		return m_DBRecordset->Fields->Item[strParamName.c_str()]->ActualSize;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return -1;
}

//��ȡ����
void CQueryResult::GetRecordsetValue(const std::string& strFieldName, CDBVarValue & DBVarValue)
{
	try
	{
		DBVarValue = m_DBRecordset->Fields->GetItem(strFieldName.c_str())->Value;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }
}

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//��ȡ����
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

//���캯��
CDataBase::CDataBase() : m_dwResumeConnectCount(30L),m_dwResumeConnectTime(30L)
{
	//״̬����
	m_dwConnectCount=0;
	m_dwConnectErrorTime=0L;

	//��������
	m_DBConnection.CreateInstance(__uuidof(Connection));

	//Ч������
	ASSERT(m_DBConnection!=NULL);
	if (m_DBConnection==NULL) throw TEXT("���ݿ����Ӷ��󴴽�ʧ��");
}

//��������
CDataBase::~CDataBase()
{
	//�ر�����
	CloseConnection();

	//�ͷŶ���
	m_DBConnection.Release();
	return;
}

//������
bool CDataBase::OpenConnection()
{
	//�������ݿ�
	try
	{
		//�ر�����
		CloseConnection();

		//�������ݿ�
		EfficacyResult(m_DBConnection->Open(_bstr_t(m_strConnect.c_str()),L"",L"",adConnectUnspecified));
		m_DBConnection->CursorLocation=adUseClient;

		//���ñ���
		m_dwConnectCount=0L;
		m_dwConnectErrorTime=0L;

		return true;
	}
	catch (CComError & ComError) { SetErrorInfo(ErrorType_Other,GetComErrorDescribe(ComError)); }

	return false;
}

//�ر�����
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

//��������
bool CDataBase::TryConnectAgain(bool bFocusConnect, CComError * pComError)
{
	try
	{
		//�ж�����
		bool bReConnect=bFocusConnect;
		if (bReConnect==false)
		{
			DWORD dwNowTime=(DWORD)time(NULL);
			if ((m_dwConnectErrorTime+m_dwResumeConnectTime)>dwNowTime) bReConnect=true;
		}
		if ((bReConnect==false)&&(m_dwConnectCount>m_dwResumeConnectCount)) bReConnect=true;

		//���ñ���
		m_dwConnectCount++;
		m_dwConnectErrorTime=(DWORD)time(NULL);
		if (bReConnect==false)
		{
			if (pComError!=NULL) SetErrorInfo(ErrorType_Connect,GetComErrorDescribe(*pComError));
			return false;
		}

		//��������
		OpenConnection();
		return true;
	}
	catch (CComError & ComError)
	{
		//�������Ӵ���
		SetErrorInfo(ErrorType_Connect,GetComErrorDescribe(ComError));
	}

	return false;
}

void CDataBase::SetConnectionInfo(const std::string& strDBPath, const std::string& strPass)
{	
	m_strConnect = std::move(strFormat("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;Jet OLEDB:Database Password=%s;",strDBPath.c_str(),strPass.c_str()));
}

//�Ƿ����Ӵ���
bool CDataBase::IsConnectError()
{
	try 
	{
		//״̬�ж�
		if (m_DBConnection==NULL) return true;
		if (m_DBConnection->GetState()==adStateClosed) return true;

		//�����ж�
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

//ת�������ַ�����ֹSQLע��
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

//ִ�����
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

//��ѯ���
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

//��ʼ����
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

//�ύ����
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

//�ع�����
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

//��ȡ����
std::string CDataBase::GetComErrorDescribe(CComError & ComError)
{
	_bstr_t bstrDescribe(ComError.Description());
	m_strErrorDescribe = std::move(strFormat("ADO ����0x%8x��%s",ComError.Error(),(char*)bstrDescribe));
	return m_strErrorDescribe;
}

//���ô���
void CDataBase::SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe)
{
	m_ADOError.SetErrorInfo(enErrorType,strDescribe);
	return;
}

//////////////////////////////////////////////////////////////////////////

//���캯��
CDataBaseEngine::CDataBaseEngine(void)
{
	//���ñ���
	m_bService=false;
	m_pDataBaseSink=NULL;

	//��ʼ�� COM
	CoInitialize(NULL);

	return;
}

//��������
CDataBaseEngine::~CDataBaseEngine(void)
{
	//ж�� COM
	CoUninitialize();
}

//ע��ӿ�
void CDataBaseEngine::SetDataBaseSink(CDataBaseSink *pDataBaseSink)
{
	//Ч�����
	ASSERT(pDataBaseSink!=NULL);

	//���ýӿ�
	m_pDataBaseSink = pDataBaseSink;
}

//��ȡ�ӿ�
CQueueService* CDataBaseEngine::GetQueueService()
{
	return &m_RequestQueueService;
}

//��������
bool CDataBaseEngine::BeginService()
{
	//�ж�״̬
	if (m_bService==true) 
	{
		return true;
	}

	//��ҽӿ�
	if (m_pDataBaseSink==NULL)
	{
		return false;
	}

	//���ö���
	m_RequestQueueService.SetQueueServiceSink(static_cast<CQueueServiceSink*>(this));

	//�������
	if (m_pDataBaseSink->BeginService()==false)
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
bool CDataBaseEngine::EndService()
{
	//���ñ���
	m_bService=false;

	//ֹͣ�������
	m_RequestQueueService.EndService();

	//ֹͣ���
	if (m_pDataBaseSink!=NULL)
	{
		m_pDataBaseSink->EndService();
	}

	return true;
}

//���нӿ�
void CDataBaseEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	//�ж�״̬
	if (m_bService==false) return;

	//������
	switch (wIdentifier)
	{
	case EVENT_DATABASE:
		{
			//Ч�����
			ASSERT(pBuffer!=NULL);
			ASSERT(wDataSize>=sizeof(NTY_DataBaseEvent));
			if (wDataSize<sizeof(NTY_DataBaseEvent)) return;

			//��������
			NTY_DataBaseEvent * pDataBaseEvent=(NTY_DataBaseEvent *)pBuffer;
			pDataBaseEvent->pDataBuffer=pDataBaseEvent+1;

			//��������
			ASSERT(m_pDataBaseSink!=NULL);
			m_pDataBaseSink->OnDataBaseRequest(*pDataBaseEvent);

			return;
		}
	}

	return;
}

//////////////////////////////////////////////////////////////////////////