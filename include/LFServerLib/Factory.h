#pragma once

#include "ServiceThread.h"

template <class T>
class CObjectFactory
{
	typedef	std::unordered_set<T*>				ObjectSet;
public:
	CObjectFactory(){}
	~CObjectFactory()
	{
		for(ObjectSet::iterator iter = m_ObjectFreeSet.begin();
			iter != m_ObjectFreeSet.end(); ++iter)
		{
			delete *iter;
		}
		for(ObjectSet::iterator iter = m_ObjectActiveSet.begin();
			iter != m_ObjectActiveSet.end(); ++iter)
		{
			delete *iter;
		}
	}

	inline T* GetFreeObject()
	{
		CThreadLockHandle Lock(&m_FactoryLock);
		T *pObject = NULL;
		if(0 == m_ObjectFreeSet.size())
		{
			pObject = new T(this);
		}
		else
		{
			ObjectSet::iterator iter = m_ObjectFreeSet.begin();
			pObject = *iter;
			m_ObjectFreeSet.erase(iter);
		}
		return pObject;
	}

	inline void InsertActiveObject(T* pObject)
	{
		if(NULL == pObject) return;

		CThreadLockHandle Lock(&m_FactoryLock);
		m_ObjectActiveSet.insert(pObject);
	}

	inline void ReleaseObject(T* pObject)
	{
		if(NULL == pObject) return;

		CThreadLockHandle Lock(&m_FactoryLock);
		ObjectSet::iterator iter = m_ObjectActiveSet.find(pObject);

		if(iter != m_ObjectActiveSet.end())
		{
			m_ObjectFreeSet.insert(*iter);
			m_ObjectActiveSet.erase(iter);
		}
	}

private:
	CThreadLock			m_FactoryLock;
	ObjectSet			m_ObjectFreeSet;
	ObjectSet			m_ObjectActiveSet;
};