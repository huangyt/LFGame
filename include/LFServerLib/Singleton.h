/*
* 文件        : Singleton.h
* 版本        : 1.0
* 描述        : 多线程单件模板类
* MM/DD/YYYY  Version    Author        Descriprion
* ------------------------------------------------
* 07/26/2010  1.0        李峰           创建
* 
*/
#pragma once
//头文件引用
#include "ServiceThread.h"

//////////////////////////////////////////////////////////////////////////

//单件模板类
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