/*
* �ļ�        : Singleton.h
* �汾        : 1.0
* ����        : ���̵߳���ģ����
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/26/2010  1.0        ���           ����
* 
*/
#pragma once
//ͷ�ļ�����
#include "ServiceThread.h"

//////////////////////////////////////////////////////////////////////////

//����ģ����
template <class T>
class Singleton
{
	CLASS_UNCOPYABLE(Singleton)
public:
	static T& Instance()
	{
		if(NULL == m_instance)
		{
			CThreadLockHandle pLockHandle(&m_Lock);
			if(NULL == m_instance)
			{
				m_instance = new T();
				atexit(Destroy);
			}
		}
		return *m_instance;
	}

protected:
	Singleton(){}
	~Singleton(){}

private:
	static void Destroy()
	{
		SAFE_DELETE(m_instance);
	}
	static CThreadLock m_Lock;
	static T* volatile m_instance;
};

template<class T>
CThreadLock Singleton<T>::m_Lock;

template<class T>
T* volatile Singleton<T>::m_instance = NULL;


//////////////////////////////////////////////////////////////////////////