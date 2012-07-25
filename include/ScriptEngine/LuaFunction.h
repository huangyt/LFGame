#pragma once

#include "LuaProxy.h"
#include <string>

template <typename RT>
class LuaFunction
{
public:
	LuaFunction(lua_State* pLuaState, const std::string strFunName, const std::string strTableName = "")
		:m_strFunName(strFunName)
		,m_strTableName(strTableName)
		,m_pLuaState(pLuaState)
	{}
	~LuaFunction(){}

	const std::string& getLastErr() const
	{
		return m_strLastError;
	}

	RT operator () ()
	{
		if(int nStartStackNum = GetLuaFun() != -1)
			return DispatchLuaFun(0, 1, nStartStackNum);
		else
			return NULL;
	}

	template <typename T>
	RT operator () (T t)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t);

			return DispatchLuaFun(1, 1, nStartStackNum);
		}
		else
		{
			return NULL;
		}
	}

	template <typename T1, typename T2>
		RT operator () (T1 t1, T2 t2)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);

			return DispatchLuaFun(2, 1, nStartStackNum);
		}
		else
		{
			return NULL;
		}
	}

	template <typename T1, typename T2, typename T3>
		RT operator () (T1 t1, T2 t2, T3 t3)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);
			CPP2Lua_Push(m_pLuaState, t3);

			return DispatchLuaFun(3, 1, nStartStackNum);
		}
		else
		{
			return NULL;
		}
	}

	template <typename T1, typename T2, typename T3, typename T4>
		RT operator () (T1 t1, T2 t2, T3 t3, T4 t4)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);
			CPP2Lua_Push(m_pLuaState, t3);
			CPP2Lua_Push(m_pLuaState, t4);

			return DispatchLuaFun(4, 1, nStartStackNum);
		}
		else
		{
			return NULL;
		}
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
		RT operator () (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);
			CPP2Lua_Push(m_pLuaState, t3);
			CPP2Lua_Push(m_pLuaState, t4);
			CPP2Lua_Push(m_pLuaState, t5);

			return DispatchLuaFun(5, 1, nStartStackNum);
		}
		else
		{
			return NULL;
		}
	}

private:
	void CleanStack(int nStartStackNum)
	{
		lua_pop(m_pLuaState, lua_gettop(m_pLuaState) - nStartStackNum);
	}

	int GetLuaFun()
	{
		int nStartStackNum = lua_gettop(m_pLuaState);

		if(!m_strTableName.empty())
		{
			lua_getglobal(m_pLuaState, m_strTableName.c_str());
			if(lua_istable(m_pLuaState, -1))
			{
				lua_pushstring(m_pLuaState, m_strFunName.c_str());
				lua_gettable(m_pLuaState, -2);
			}
		}	
		else
		{
			lua_getglobal(m_pLuaState, m_strFunName.c_str());
		}

		if(!lua_isfunction(m_pLuaState, -1))
		{
			m_strLastError = "error:not function!";
			CleanStack(nStartStackNum);
			return -1;
		}
		else
		{
			return nStartStackNum;
		}
	}

	RT DispatchLuaFun(int nArgs, int nResults, int nStartStackNum)
	{
		if(0 != lua_pcall(m_pLuaState, nArgs, nResults, 0))
		{
			m_strLastError = lua_tostring(m_pLuaState,-1);
			CleanStack(nStartStackNum);
			return NULL;
		}
		else
		{
			m_strLastError = "";
			RT temp = Lua2CPP_TypeMatch(m_pLuaState, -1, temp);
			CleanStack(nStartStackNum);
			return temp;
		}
	}

private:
	std::string m_strFunName;
	std::string m_strTableName;
	std::string m_strLastError;
	lua_State	*m_pLuaState;
};

template <>
class LuaFunction<void>
{
public:
	LuaFunction(lua_State* pLuaState, const std::string strFunName, const std::string strTableName = "")
		:m_strFunName(strFunName)
		,m_strTableName(strTableName)
		,m_pLuaState(pLuaState)
	{}
	~LuaFunction(){}

	const std::string& getLastErr() const
	{
		return m_strLastError;
	}

	void operator () ()
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			DispatchLuaFun(0, 0, nStartStackNum);
		}
	}

	template <typename T>
		void operator () (T t)
	{
	if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t);

			DispatchLuaFun(1, 0, nStartStackNum);
		}
	}

	template <typename T1, typename T2>
		void operator () (T1 t1, T2 t2)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);

			DispatchLuaFun(2, 0, nStartStackNum);
		}
	}

	template <typename T1, typename T2, typename T3>
		void operator () (T1 t1, T2 t2, T3 t3)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);
			CPP2Lua_Push(m_pLuaState, t3);

			DispatchLuaFun(3, 0, nStartStackNum);
		}
	}

	template <typename T1, typename T2, typename T3, typename T4>
		void operator () (T1 t1, T2 t2, T3 t3, T4 t4)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);
			CPP2Lua_Push(m_pLuaState, t3);
			CPP2Lua_Push(m_pLuaState, t4);

			DispatchLuaFun(4, 0, nStartStackNum);
		}
	}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
		void operator () (T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
	{
		if(int nStartStackNum = GetLuaFun() != -1)
		{
			CPP2Lua_Push(m_pLuaState, t1);
			CPP2Lua_Push(m_pLuaState, t2);
			CPP2Lua_Push(m_pLuaState, t3);
			CPP2Lua_Push(m_pLuaState, t4);
			CPP2Lua_Push(m_pLuaState, t5);

			DispatchLuaFun(5, 0, nStartStackNum);
		}
	}

private:
	void CleanStack(int nStartStackNum)
	{
		lua_pop(m_pLuaState, lua_gettop(m_pLuaState) - nStartStackNum);
	}

	int GetLuaFun()
	{
		int nStartStackNum = lua_gettop(m_pLuaState);

		if(!m_strTableName.empty())
		{
			lua_getglobal(m_pLuaState, m_strTableName.c_str());
			if(lua_istable(m_pLuaState, -1))
			{
				lua_pushstring(m_pLuaState, m_strFunName.c_str());
				lua_gettable(m_pLuaState, -2);
			}
		}	
		else
		{
			lua_getglobal(m_pLuaState, m_strFunName.c_str());
		}

		if(!lua_isfunction(m_pLuaState, -1))
		{
			m_strLastError = "error:not function!";
			CleanStack(nStartStackNum);
			return -1;
		}
		else
		{
			return nStartStackNum;
		}
	}

	void DispatchLuaFun(int nArgs, int nResults, int nStartStackNum)
	{
		if(0 != lua_pcall(m_pLuaState, nArgs, nResults, 0))
		{
			m_strLastError = lua_tostring(m_pLuaState,-1);
			CleanStack(nStartStackNum);
		}
		else
		{
			m_strLastError = "";
		}
	}

private:
	std::string m_strFunName;
	std::string m_strTableName;
	std::string m_strLastError;
	lua_State	*m_pLuaState;
};
