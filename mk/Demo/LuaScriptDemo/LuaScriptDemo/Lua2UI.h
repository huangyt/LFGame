#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include "lua.hpp"
#include <assert.h>

#include <Windows.h>

#include <LuaPointer.h>
#include <ScriptEngine.h>
#include <LuaProxy.h>

#define pt(T) std::cout<<T<<std::endl

class EventPip
{
public:
	EventPip(){}
	~EventPip()
	{
		for(std::map<int, std::queue<int>* >::iterator iter = m_EventMap.begin();
			iter != m_EventMap.end(); ++iter)
		{
			while(!iter->second->empty())
			{
				iter->second->pop();
			}

			delete iter->second;
		}
		m_EventMap.clear();
	}

	static void push(int nDlgID, int nID)
	{
		if(m_EventMap.find(nDlgID) != m_EventMap.end())
		{
			m_EventMap[nDlgID]->push(nID);
		}
		else
		{
			std::queue<int>* pTempQuqeue = new std::queue<int>;
			pTempQuqeue->push(nID);
			m_EventMap[nDlgID] = pTempQuqeue;
		}
	}

	static int pop(int nDlgID)
	{
		if(m_EventMap.find(nDlgID) != m_EventMap.end())
		{
			int nTemp = -1;
			if(m_EventMap[nDlgID]->size() > 0)
			{
				nTemp = m_EventMap[nDlgID]->front();
				m_EventMap[nDlgID]->pop();
			}
			return nTemp;
		}
		else
			return -1;
	}

	static short GetKS(int vKey)
	{
		return GetAsyncKeyState(vKey);
	}

public:
	static std::map<int, std::queue<int>* > m_EventMap;
};

class Dialog
{
	typedef struct _DlgCtr
	{
		int			_nID;
		const char* _strCtr;
	}DlgCtr;
public:
	Dialog(lua_State *L)
		:m_nIndex(-1)
		,m_nDlgID(-1)
		,m_pState(L)
	{
		int a  = lua_tointeger(L, 1);

	}
	~Dialog()
	{
		m_Choices.clear();
		pt("对话框析构");
	}
	void setID(int nID)
	{
		m_nDlgID = nID;
		//LuaPointer::pushOnToStack(m_pState, &m_nDlgID);
	}
	void setID()
	{
		pt("test");
	}
	int getID(int nID,void* pDlg)
	{
		return m_nDlgID;
	}

	int setIDPtr(lua_State *L)
	{
		LuaPointer::pushOnToStack(m_pState, &m_nDlgID);
		return 1;
	}

// 	int test(lua_State *L)
// 	{
// 		float *a = LuaPointer::checkPointer<float>(L, 1);
// 		return 0;
// 	}

	int test(lua_State *L)
	{
		int a  = lua_tointeger(L, 1);
		//int a = Lua2CPP_TypeMatch(L, 1, a);
		CPP2Lua_Push(L, a);
		return 1;
	}

	void addChoice(const char *strChoice)
	{
		assert(m_nDlgID != -1 && "窗口没有设置ID");
		++m_nIndex;
		DlgCtr temp;
		memset(&temp, 0, sizeof(DlgCtr));
		temp._nID = m_nIndex;
		temp._strCtr = strChoice;
		m_Choices.push_back(temp);
	}
	void startDialog()
	{
		if(m_nIndex>-1)
		{
			for(int i = 0; i <= m_nIndex; ++i)
			{
				pt(m_Choices[i]._strCtr);
			}
		}
	}
private:
	int m_nIndex;
	int m_nDlgID;
	lua_State*	m_pState;
	std::vector<DlgCtr> m_Choices;
};
