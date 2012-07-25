#ifndef __LUA_TABLE_H__
#define __LUA_TABLE_H__

#include "LuaArray.h"

class LuaTable
{
public:
	static bool		isGlobalFieldNil(lua_State *L, const char *k)
	{
		lua_getglobal(L, k);
		bool b = lua_isnil(L, -1);
		lua_pop(L, 1);
		return b;
	}

	template <typename T>
		static bool		getGlobalField(lua_State *L, const char *k, T &r)
	{
		bool bFlag = false;
		lua_getglobal(L, k);	// v
		if(!lua_isnil(L, -1))
		{
			r = Lua2CPP_TypeMatch(L, -1, r);
			bFlag = true;
		}

		lua_pop(L, 1);	// pop v
		return bFlag;
	}

	template <typename T>
		static void		setGlobalField(lua_State *L, const char *k, T &v)
	{
		CPP2Lua_Push(L, v); // v
		lua_setfield(L, LUA_GLOBALSINDEX, k);  // this function shall pop v
	}

	bool		isNil(const char *k)
	{
		lua_getglobal(m_pLuaState, m_csName);
		if(lua_istable(m_pLuaState, -1))
		{
			lua_pushstring(m_pLuaState, k);
			lua_gettable(m_pLuaState, -2);
			bool b = lua_isnil(m_pLuaState, -1);
			lua_pop(m_pLuaState, 2);
			return b;
		}

		lua_pop(m_pLuaState, 1);
		return false;
	}

	ArrayPrimNode	getRaw(int idx)
	{
		ArrayPrimNode node;
		node.type = LUA_TNIL;
		node.data.dl = 0;
		lua_getglobal(m_pLuaState, m_csName);
		if(lua_istable(m_pLuaState, -1))
		{
			lua_rawgeti(m_pLuaState, -1, idx);
			luaStackToArrayPrimNode(m_pLuaState, -1, node);
			lua_pop(m_pLuaState, 1);
		}
		lua_pop(m_pLuaState, 1);

		return node;
	}


	template <typename T>
		void	setRaw(int idx, const T &v)
		{
			lua_getglobal(m_pLuaState, m_csName);
			if(lua_istable(m_pLuaState, -1))
			{
				CPP2Lua_Push(m_pLuaState, v);
				lua_rawseti(m_pLuaState, -2, idx);
			}
			lua_pop(m_pLuaState, 1);
		}

	// 获取表项
	template <typename T>
		bool		getField(const char *k, T &r)
	{
		bool bFlag = false;
		lua_getglobal(m_pLuaState, m_csName);
		if(lua_istable(m_pLuaState, -1))
		{
			lua_pushstring(m_pLuaState, k);
			lua_gettable(m_pLuaState, -2);
			if(!lua_isnil(m_pLuaState, -1))
			{
				r = Lua2CPP_TypeMatch(m_pLuaState, -1, r);
				bFlag = true;
			}
			lua_pop(m_pLuaState, 1);
		}
		lua_pop(m_pLuaState, 1);

		return bFlag;
	}

	// 设置表项
	template <typename T>
		void	setField(const char *k, T &v)
	{
		lua_getglobal(m_pLuaState, m_csName);	// t
		if(lua_istable(m_pLuaState, -1))
		{
			lua_pushstring(m_pLuaState, k);	// t k
			CPP2Lua_Push(m_pLuaState, v);	// t k v
			lua_settable(m_pLuaState, -3);	// t
		}
		lua_pop(m_pLuaState, 1);	// pop t
	}

	//! 创建表快照以支持安全迭代
	void	makeTableSnapshot(std::vector<ArrayPrimNode> &snapshot)
	{
		int nts = lua_gettop(m_pLuaState);
		lua_getglobal(m_pLuaState, m_csName);	// t
		if(lua_istable(m_pLuaState, -1))
		{
			lua_pushnil(m_pLuaState); // t n
			while(lua_next(m_pLuaState, -2) != 0)
			{
				// t k v
				ArrayPrimNode node;
				luaStackToArrayPrimNode(m_pLuaState, -1, node);
				lua_pop(m_pLuaState, 1);
				snapshot.push_back(node);
			}
		}
		int nte = lua_gettop(m_pLuaState);
		lua_pop(m_pLuaState, nte - nts);
	}

	static void luaStackToArrayPrimNode(lua_State *L, int idx, ArrayPrimNode &node)
	{
		int tt = lua_type(L, idx);
		switch(tt)
		{
		case LUA_TNUMBER:
			node.data.dl = lua_tonumber(L, idx);
			node.type = LUA_TNUMBER;
			break;
		case LUA_TBOOLEAN:
			node.data.dl = lua_toboolean(L, idx);
			node.type = LUA_TBOOLEAN;
			break;
		case LUA_TSTRING:
			node.data.ptr = (void*)lua_tostring(L, idx);
			node.type = LUA_TSTRING;
			break;
		case LUA_TLIGHTUSERDATA:
			node.data.ptr = lua_touserdata(L, idx);
			node.type = LUA_TLIGHTUSERDATA;
			break;
		}
	}
	
	//! name & lua_State must be valid
	LuaTable(const char *szName, lua_State *L)
		:m_csName(szName), m_pLuaState(L)
	{
	}

private:
	const char *m_csName;
	lua_State *m_pLuaState;
};

#endif //end __LUA_TABLE_H__