/*
* 文件        : DataStorage.h
* 版本        : 1.0
* 描述        : 数据存储接口声明
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        李峰           创建
* 
*/
#pragma once

//头文件引用
#include "GlobalDef.h"

//////////////////////////////////////////////////////////////////////////
//结构体定义

//数据队列头
struct tagDataHead
{
	WORD							wDataSize;							//数据大小
	WORD							wIdentifier;						//类型标识
};

//负荷信息
struct tagBurthenInfo
{
	DWORD							dwDataSize;							//数据大小
	DWORD							dwBufferSize;						//缓冲区长度
	DWORD							dwDataPacketCount;					//数据包数目
};

//////////////////////////////////////////////////////////////////////////

//数据存储类
class CDataStorage
{
	//函数定义
public:
	//构造函数
	CDataStorage(void);
	//析构函数
	~CDataStorage(void);

	//功能函数
public:
	//负荷信息
	bool GetBurthenInfo(tagBurthenInfo & BurthenInfo);
	//插入数据
	bool AddData(WORD wIdentifier, void * const pBuffer, WORD wDataSize);
	//获取数据
	bool GetData(tagDataHead & DataHead, void * pBuffer, WORD wBufferSize);
	//删除数据
	void RemoveData(bool bFreeMemroy);

	//查询变量
protected:
	DWORD							m_dwInsertPos;						//插入位置
	DWORD							m_dwTerminalPos;					//结束位置
	DWORD							m_dwDataQueryPos;					//查询位置

	//数据变量
protected:
	DWORD							m_dwDataSize;						//数据大小
	DWORD							m_dwDataPacketCount;				//数据包数

	//缓冲变量
protected:
	DWORD							m_dwBufferSize;						//缓冲长度
	LPBYTE							m_pDataStorageBuffer;				//缓冲指针
};

//////////////////////////////////////////////////////////////////////////
