/*
* �ļ�        : DataStorage.h
* �汾        : 1.0
* ����        : ���ݴ洢�ӿ�����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/19/2010  1.0        ���           ����
* 
*/
#pragma once

//ͷ�ļ�����
#include "GlobalDef.h"

//////////////////////////////////////////////////////////////////////////
//�ṹ�嶨��

//���ݶ���ͷ
struct tagDataHead
{
	WORD							wDataSize;							//���ݴ�С
	WORD							wIdentifier;						//���ͱ�ʶ
};

//������Ϣ
struct tagBurthenInfo
{
	DWORD							dwDataSize;							//���ݴ�С
	DWORD							dwBufferSize;						//����������
	DWORD							dwDataPacketCount;					//���ݰ���Ŀ
};

//////////////////////////////////////////////////////////////////////////

//���ݴ洢��
class CDataStorage
{
	//��������
public:
	//���캯��
	CDataStorage(void);
	//��������
	~CDataStorage(void);

	//���ܺ���
public:
	//������Ϣ
	bool GetBurthenInfo(tagBurthenInfo & BurthenInfo);
	//��������
	bool AddData(WORD wIdentifier, void * const pBuffer, WORD wDataSize);
	//��ȡ����
	bool GetData(tagDataHead & DataHead, void * pBuffer, WORD wBufferSize);
	//ɾ������
	void RemoveData(bool bFreeMemroy);

	//��ѯ����
protected:
	DWORD							m_dwInsertPos;						//����λ��
	DWORD							m_dwTerminalPos;					//����λ��
	DWORD							m_dwDataQueryPos;					//��ѯλ��

	//���ݱ���
protected:
	DWORD							m_dwDataSize;						//���ݴ�С
	DWORD							m_dwDataPacketCount;				//���ݰ���

	//�������
protected:
	DWORD							m_dwBufferSize;						//���峤��
	LPBYTE							m_pDataStorageBuffer;				//����ָ��
};

//////////////////////////////////////////////////////////////////////////
