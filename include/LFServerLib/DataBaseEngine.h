/*
* 文件        : DataBaseEngine.h
* 版本        : 1.0
* 描述        : 数据库引擎接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/22/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件引用
#include "GlobalDef.h"
#include "QueueService.h"
#include "Factory.h"

//////////////////////////////////////////////////////////////////////////

//ADO 导入库
#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","adoEOF")

//COM 错误类型
typedef _com_error					CComError;							//COM 错误
typedef _variant_t					CDBVarValue;						//数据库数值

//数据库错误代码
enum enADOErrorType
{
	ErrorType_Nothing				=0,									//没有错误
	ErrorType_Connect				=1,									//连接错误
	ErrorType_Other					=2,									//其他错误
};

//////////////////////////////////////////////////////////////////////////

//ADO 错误类
class CADOError
{
	//函数定义
public:
	//构造函数
	CADOError();
	//析构函数
	virtual ~CADOError();

	//功能接口
public:
	//错误类型
	virtual enADOErrorType GetErrorType() { return m_enErrorType; }
	//错误描述
	virtual std::string GetErrorDescribe() { return m_strErrorDescribe; }

	//功能函数
public:
	//设置错误
	void SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe);

	//变量定义
protected:
	enADOErrorType					m_enErrorType;						//错误代号
	std::string						m_strErrorDescribe;					//错误信息
};

//////////////////////////////////////////////////////////////////////////

//时间定义类
class CDateTime
{
	//函数定义
public:
	//时间状态枚举
	enum DateTimeStatus
	{
		valid = 1,
		null = 2,
	};

	//构造函数
	CDateTime():m_dt( 0 ),m_status(valid){}

	//操作函数
public:
	//设置时间
	void SetTime(DATE datetime)
	{
		m_dt = datetime;
	}

	//设置为NULL
	void SetNULL()
	{
		m_status = null;
		m_dt = 0;
	}

	//转换成时间格式
	void GetAsSystemTime(SYSTEMTIME& sysTime)
	{
		::VariantTimeToSystemTime(m_dt, &sysTime);
	}
	//变量定义
private:
	//内部数据
	DateTimeStatus	m_status;			//时间状态
	DATE			m_dt;				//时间数据
};

//////////////////////////////////////////////////////////////////////////
//数据集对象
class CQueryResult
{
	friend class CDataBase;
	friend class CObjectFactory<CQueryResult>;
public:
	//删除
	void Release();

	//记录集接口
public:
	//往下移动
	void MoveToNext();
	//移到开头
	void MoveToFirst();
	//是否结束
	bool IsEndRecordset();
	//获取数目
	LONG GetRecordCount();
	//获取大小
	LONG GetActualSize(const std::string& strParamName);
	//获取数据
	void GetRecordsetValue(const std::string& strFieldName, CDBVarValue & DBVarValue);

	//字段接口
public:
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, BYTE & bValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, WORD & wValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, INT & nValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, LONG & lValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, DWORD & ulValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, UINT & ulValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, DOUBLE & dbValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, __int64 & llValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, std::string & strValue);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, CDateTime & Time);
	//获取参数
	bool GetFieldValue(const std::string& strFieldName, bool & bValue);

	//内部函数
protected:
	CQueryResult(CObjectFactory<CQueryResult>* pFactory);
	virtual ~CQueryResult();

	//关闭记录
	bool CloseRecordset();

	//是否打开
	bool IsRecordsetOpened();

	//加载数据集
	void Attach(const _RecordsetPtr& ptr);

	//获取错误
	std::string GetComErrorDescribe(CComError & ComError);
	//设置错误
	void SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe);

protected:
	_RecordsetPtr					m_DBRecordset;						//记录集对象
	CADOError						m_ADOError;							//错误对象
	std::string						m_strErrorDescribe;					//错误信息
	CObjectFactory<CQueryResult>	*m_pObjectFactory;
};

//////////////////////////////////////////////////////////////////////////

//数据库对象
class CDataBase
{
	//函数定义
public:
	//构造函数
	CDataBase();
	//析构函数
	~CDataBase();

	//管理接口
public:
	//打开连接
	bool OpenConnection();
	//关闭连接
	bool CloseConnection();
	//重新连接
	bool TryConnectAgain(bool bFocusConnect, CComError * pComError);
	//设置信息
	void SetConnectionInfo(const std::string& strDBPath, const std::string& strPass);

	//状态接口
public:
	//是否连接错误
	bool IsConnectError();

	//执行接口
public:
	//转义特殊字符，防止SQL注入
	void EscapeString(std::string& str);

	//执行语句
	bool Execute(const std::string& strSql);

	//查询语句
	CQueryResult* Query(const std::string& strSql);

	//开始事务
	bool BeginTransaction(void);

	//提交事务
	bool CommitTransaction(void);

	//回滚事务
	bool RollbackTransaction(void);

	//内部函数
private:
	//获取错误
	std::string GetComErrorDescribe(CComError & ComError);
	//设置错误
	void SetErrorInfo(enADOErrorType enErrorType, const std::string& strDescribe);

	//信息变量
protected:
	CADOError						m_ADOError;							//错误对象
	std::string						m_strConnect;						//连接字符串
	std::string						m_strErrorDescribe;					//错误信息

	//状态变量
protected:
	DWORD							m_dwConnectCount;					//重试次数
	DWORD							m_dwConnectErrorTime;				//错误时间
	const DWORD						m_dwResumeConnectCount;				//恢复次数
	const DWORD						m_dwResumeConnectTime;				//恢复时间

	//内核变量
protected:
	_ConnectionPtr						m_DBConnection;					//数据库对象
	CThreadLock							m_DBLock;						//数据库访问锁
	static CObjectFactory<CQueryResult>	m_QueryResultFactory;
};

//////////////////////////////////////////////////////////////////////////

//数据库钩子接口
class CDataBaseSink
{
public:
	//数据库模块启动
	virtual bool BeginService()=NULL;
	//数据库模块关闭
	virtual bool EndService()=NULL;
	//数据操作处理
	virtual bool OnDataBaseRequest(const NTY_DataBaseEvent & DataBaseEvent)=NULL;
};

//////////////////////////////////////////////////////////////////////////

//数据库管理类
class CDataBaseEngine : public CQueueServiceSink
{
	//函数定义
public:
	//构造函数
	CDataBaseEngine(void);
	//析构函数
	~CDataBaseEngine(void);

	//服务接口
public:
	//启动服务
	bool BeginService();
	//停止服务
	bool EndService();
	//注册钩子
	void SetDataBaseSink(CDataBaseSink *pDataBaseSink);
	//获取接口
	CQueueService* GetQueueService();

	//队列接口
public:
	//队列接口
	virtual void OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize);

	//内核变量
protected:
	bool							m_bService;							//运行标志
	CQueueService					m_RequestQueueService;				//队列对象
	CDataBaseSink					*m_pDataBaseSink;					//通知钩子
};

//////////////////////////////////////////////////////////////////////////