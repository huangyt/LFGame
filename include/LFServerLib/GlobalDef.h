/*
* 文件        : GlobalDef.h
* 版本        : 1.0
* 描述        : 全局定义
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        李峰           创建
* 
*/

#pragma once

//头文件引用
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <time.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>

#define ASSERT	assert

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }

#define SafeDeleteArray(pData)	{ try { delete [] pData; } catch (...) { } pData=NULL; } 

#define UINT64_LITERAL(n)			n ## ui64
#define INT64_LITERAL(n)			n ## i64

//网络数据定义
#define SOCKET_VER					0x0001								//数据包版本
#define SOCKET_BUFFER				10240								//最大网络包
#define SOCKET_PACKET				(SOCKET_BUFFER-sizeof(CMD_Head)-2*sizeof(DWORD))

//网络检测
#define CMD_DETECT_SOCKET			0x00000000							//检测命令

//常量宏定义
#define MAX_QUEUE_PACKET			10240								//最大队列
#define COMMAND_AUTHCRYPT_KEY		"D0118BFC8E3D5096A71267179DAAD487"	//加密密钥

//////////////////////////////////////////////////////////////////////////
//事件定义

//事件标识
#define EVENT_TIMER					0x0001								//定时器引擎
#define EVENT_DATABASE				0x0002								//数据库请求
#define EVENT_SOCKET_ACCEPT			0x0003								//网络应答
#define EVENT_SOCKET_READ			0x0004								//网络读取
#define EVENT_SOCKET_CLOSE			0x0005								//网络关闭

//数据包结构信息
struct CMD_Info
{
	WORD							wDataSize;							//数据大小
	BYTE							cbCheckCode;						//效验字段
	BYTE							cbMessageVer;						//版本标识
};

//数据包传递包头
struct CMD_Head
{
	CMD_Info						CmdInfo;							//基础结构
	DWORD							dwCommand;							//命令信息
};

//定时器事件
struct NTY_TimerEvent
{
	WORD							wTimerID;							//定时器 ID
	WPARAM							wBindParam;							//绑定参数
};

//数据库请求事件
struct NTY_DataBaseEvent
{
	DWORD							dwSocketID;							//Socket编号
	WORD							wRequestID;							//请求标识
	WORD							wDataSize;							//数据大小
	LPVOID							pDataBuffer;						//数据地址
};

//网络应答事件
struct NTY_SocketAcceptEvent
{
	DWORD							dwSocketID;							//Socket编号
	DWORD							dwClientIP;							//连接地址
};

//网络读取事件
struct NTY_SocketReadEvent
{
	DWORD							dwSocketID;							//Socket编号
	DWORD							dwCommand;							//命令信息
	WORD							wDataSize;							//数据大小
	LPVOID							pDataBuffer;						//数据地址
};

//网络关闭事件
struct NTY_SocketCloseEvent
{
	DWORD							dwSocketID;							//Socket编号
	DWORD							dwClientIP;							//连接地址
	DWORD							dwConnectSecond;					//连接时间
};

//检测结构信息
struct CMD_KN_DetectSocket
{
	DWORD							dwSendTickCount;					//发送时间
	DWORD							dwRecvTickCount;					//接收时间
};