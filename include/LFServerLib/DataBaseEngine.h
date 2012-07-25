/*
* �ļ�        : DataBaseEngine.h
* �汾        : 1.0
* ����        : ���ݿ�����ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/22/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"
#include "QueueService.h"
#include "Factory.h"

//////////////////////////////////////////////////////////////////////////

//ADO �����
#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","adoEOF")

//COM ��������
typedef _com_error					CComError;							//COM ����
typedef _variant_t					CDBVarValue;						//���ݿ���ֵ

//���ݿ�������
enum enADOErrorType
{
	ErrorType_Nothing				=0,									//û�д���
	ErrorType_Connect				=1,									//���Ӵ���
	ErrorType_Other					=2,									//��������
};

//////////////////////////////////////////////////////////////////////////

//ADO ������
class CADOError
{
	//��������
public:
	//���캯��
	CADOError();
	//��������
	virtual ~CADOError();

	//���ܽӿ�
public:
	//��������
	virtual enADOErrorType GetErrorType() { return m_enErrorType; }
	//��������
	virtual std::string GetErrorDescribe() { return m_strErrorDescribe; }

	//���ܺ���
public:
	//���ô���
	void SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe);

	//��������
protected:
	enADOErrorType					m_enErrorType;						//�������
	std::string						m_strErrorDescribe;					//������Ϣ
};

//////////////////////////////////////////////////////////////////////////

//ʱ�䶨����
class CDateTime
{
	//��������
public:
	//ʱ��״̬ö��
	enum DateTimeStatus
	{
		valid = 1,
		null = 2,
	};

	//���캯��
	CDateTime():m_dt( 0 ),m_status(valid){}

	//��������
public:
	//����ʱ��
	void SetTime(DATE datetime)
	{
		m_dt = datetime;
	}

	//����ΪNULL
	void SetNULL()
	{
		m_status = null;
		m_dt = 0;
	}

	//ת����ʱ���ʽ
	void GetAsSystemTime(SYSTEMTIME& sysTime)
	{
		::VariantTimeToSystemTime(m_dt, &sysTime);
	}
	//��������
private:
	//�ڲ�����
	DateTimeStatus	m_status;			//ʱ��״̬
	DATE			m_dt;				//ʱ������
};

//////////////////////////////////////////////////////////////////////////
//���ݼ�����
class CQueryResult
{
	friend class CDataBase;
	friend class CObjectFactory<CQueryResult>;
public:
	//ɾ��
	void Release();

	//��¼���ӿ�
public:
	//�����ƶ�
	void MoveToNext();
	//�Ƶ���ͷ
	void MoveToFirst();
	//�Ƿ����
	bool IsEndRecordset();
	//��ȡ��Ŀ
	LONG GetRecordCount();
	//��ȡ��С
	LONG GetActualSize(const std::string& strParamName);
	//��ȡ����
	void GetRecordsetValue(const std::string& strFieldName, CDBVarValue & DBVarValue);

	//�ֶνӿ�
public:
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, BYTE & bValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, WORD & wValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, INT & nValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, LONG & lValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, DWORD & ulValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, UINT & ulValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, DOUBLE & dbValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, __int64 & llValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, std::string & strValue);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, CDateTime & Time);
	//��ȡ����
	bool GetFieldValue(const std::string& strFieldName, bool & bValue);

	//�ڲ�����
protected:
	CQueryResult(CObjectFactory<CQueryResult>* pFactory);
	virtual ~CQueryResult();

	//�رռ�¼
	bool CloseRecordset();

	//�Ƿ��
	bool IsRecordsetOpened();

	//�������ݼ�
	void Attach(const _RecordsetPtr& ptr);

	//��ȡ����
	std::string GetComErrorDescribe(CComError & ComError);
	//���ô���
	void SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe);

protected:
	_RecordsetPtr					m_DBRecordset;						//��¼������
	CADOError						m_ADOError;							//�������
	std::string						m_strErrorDescribe;					//������Ϣ
	CObjectFactory<CQueryResult>	*m_pObjectFactory;
};

//////////////////////////////////////////////////////////////////////////

//���ݿ����
class CDataBase
{
	//��������
public:
	//���캯��
	CDataBase();
	//��������
	~CDataBase();

	//����ӿ�
public:
	//������
	bool OpenConnection();
	//�ر�����
	bool CloseConnection();
	//��������
	bool TryConnectAgain(bool bFocusConnect, CComError * pComError);
	//������Ϣ
	void SetConnectionInfo(const std::string& strDBPath, const std::string& strPass);

	//״̬�ӿ�
public:
	//�Ƿ����Ӵ���
	bool IsConnectError();

	//ִ�нӿ�
public:
	//ת�������ַ�����ֹSQLע��
	void EscapeString(std::string& str);

	//ִ�����
	bool Execute(const std::string& strSql);

	//��ѯ���
	CQueryResult* Query(const std::string& strSql);

	//��ʼ����
	bool BeginTransaction(void);

	//�ύ����
	bool CommitTransaction(void);

	//�ع�����
	bool RollbackTransaction(void);

	//�ڲ�����
private:
	//��ȡ����
	std::string GetComErrorDescribe(CComError & ComError);
	//���ô���
	void SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe);

	//��Ϣ����
protected:
	CADOError						m_ADOError;							//�������
	std::string						m_strConnect;						//�����ַ���
	std::string						m_strErrorDescribe;					//������Ϣ

	//״̬����
protected:
	DWORD							m_dwConnectCount;					//���Դ���
	DWORD							m_dwConnectErrorTime;				//����ʱ��
	const DWORD						m_dwResumeConnectCount;				//�ָ�����
	const DWORD						m_dwResumeConnectTime;				//�ָ�ʱ��

	//�ں˱���
protected:
	_ConnectionPtr						m_DBConnection;					//���ݿ����
	CThreadLock							m_DBLock;						//���ݿ������
	static CObjectFactory<CQueryResult>	m_QueryResultFactory;
};

//////////////////////////////////////////////////////////////////////////

//���ݿ⹳�ӽӿ�
class CDataBaseSink
{
public:
	//���ݿ�ģ������
	virtual bool BeginService()=NULL;
	//���ݿ�ģ��ر�
	virtual bool EndService()=NULL;
	//���ݲ�������
	virtual bool OnDataBaseRequest(const NTY_DataBaseEvent & DataBaseEvent)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//���ݿ������
class CDataBaseEngine : public CQueueServiceSink
{
	//��������
public:
	//���캯��
	CDataBaseEngine(void);
	//��������
	~CDataBaseEngine(void);

	//����ӿ�
public:
	//��������
	bool BeginService();
	//ֹͣ����
	bool EndService();
	//ע�ṳ��
	void SetDataBaseSink(CDataBaseSink *pDataBaseSink);
	//��ȡ�ӿ�
	CQueueService* GetQueueService();

	//���нӿ�
public:
	//���нӿ�
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//�ں˱���
protected:
	bool							m_bService;							//���б�־
	CQueueService					m_RequestQueueService;				//���ж���
	CDataBaseSink					*m_pDataBaseSink;					//֪ͨ����
};

//////////////////////////////////////////////////////////////////////////