#pragma once

//Command���� wMainCmdID=������� wSubCmdID=������
//��ǰ�汾:0.0.1 120303

enum eCMDResult
{
	RET_SUCCESS						=	0x00000000,
	RET_FAIL_BANNED					=	0x00000001,
	RET_FAIL_UNKNOWN_ACCOUNT		=	0x00000002,
	RET_FAIL_INCORRECT_PASSWORD		=	0x00000003,

	RET_FAIL_UNKNOWN				=	0xFFFFFFFF
};

enum eLoginCMD
{
	//0x00000000�Ѿ���Socket����ռ��
	CMD_LF_LOGON_CHALLENGE			=	0x00000001,

	CMD_LF_UNKNOWN					=	0xFFFFFFFF
};

//CMD_LF_LOGON_CHALLENGE
typedef struct LF_LOGON_CHALLENGE
{
	WORD	size;				//����
	BYTE	version1;			//0
	BYTE	version2;			//0
	BYTE	version3;			//1
	DWORD	build;				//120303
	BYTE	I_len;				//����������
	BYTE	I[1];
}sLFLogonChanllenge;
