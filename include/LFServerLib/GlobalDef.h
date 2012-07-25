/*
* �ļ�        : GlobalDef.h
* �汾        : 1.0
* ����        : ȫ�ֶ���
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        ���           ����
* 
*/

#pragma once

//ͷ�ļ�����
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

//�������ݶ���
#define SOCKET_VER					0x0001								//���ݰ��汾
#define SOCKET_BUFFER				10240								//��������
#define SOCKET_PACKET				(SOCKET_BUFFER-sizeof(CMD_Head)-2*sizeof(DWORD))

//������
#define CMD_DETECT_SOCKET			0x00000000							//�������

//�����궨��
#define MAX_QUEUE_PACKET			10240								//������
#define COMMAND_AUTHCRYPT_KEY		"D0118BFC8E3D5096A71267179DAAD487"	//������Կ

//////////////////////////////////////////////////////////////////////////
//�¼�����

//�¼���ʶ
#define EVENT_TIMER					0x0001								//��ʱ������
#define EVENT_DATABASE				0x0002								//���ݿ�����
#define EVENT_SOCKET_ACCEPT			0x0003								//����Ӧ��
#define EVENT_SOCKET_READ			0x0004								//�����ȡ
#define EVENT_SOCKET_CLOSE			0x0005								//����ر�

//���ݰ��ṹ��Ϣ
struct CMD_Info
{
	WORD							wDataSize;							//���ݴ�С
	BYTE							cbCheckCode;						//Ч���ֶ�
	BYTE							cbMessageVer;						//�汾��ʶ
};

//���ݰ����ݰ�ͷ
struct CMD_Head
{
	CMD_Info						CmdInfo;							//�����ṹ
	DWORD							dwCommand;							//������Ϣ
};

//��ʱ���¼�
struct NTY_TimerEvent
{
	WORD							wTimerID;							//��ʱ�� ID
	WPARAM							wBindParam;							//�󶨲���
};

//���ݿ������¼�
struct NTY_DataBaseEvent
{
	DWORD							dwSocketID;							//Socket���
	WORD							wRequestID;							//�����ʶ
	WORD							wDataSize;							//���ݴ�С
	LPVOID							pDataBuffer;						//���ݵ�ַ
};

//����Ӧ���¼�
struct NTY_SocketAcceptEvent
{
	DWORD							dwSocketID;							//Socket���
	DWORD							dwClientIP;							//���ӵ�ַ
};

//�����ȡ�¼�
struct NTY_SocketReadEvent
{
	DWORD							dwSocketID;							//Socket���
	DWORD							dwCommand;							//������Ϣ
	WORD							wDataSize;							//���ݴ�С
	LPVOID							pDataBuffer;						//���ݵ�ַ
};

//����ر��¼�
struct NTY_SocketCloseEvent
{
	DWORD							dwSocketID;							//Socket���
	DWORD							dwClientIP;							//���ӵ�ַ
	DWORD							dwConnectSecond;					//����ʱ��
};

//���ṹ��Ϣ
struct CMD_KN_DetectSocket
{
	DWORD							dwSendTickCount;					//����ʱ��
	DWORD							dwRecvTickCount;					//����ʱ��
};