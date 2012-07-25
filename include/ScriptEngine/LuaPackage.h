#ifndef __LUA_PACKAGE_H__
#define __LUA_PACKAGE_H__

#include "LuaProxy.h"

//! 用于定义提供给lua的系统级方法包
// 旧式宏，只可注册int (*) (lua_State*)形式
#define LUA_PACKAGE_BEGIN(pak) static const luaL_reg s_##pak##_Reg[] = {

#define LUA_PACKAGE_FUNC(func_name, func) { #func_name, func }

#define LUA_PACKAGE_END { 0, 0 } };

#define REGISTER_LUA_PACKAGE(L, pak) luaL_register(L, #pak, s_##pak##_Reg)

//! 任意形式API包注册器
class LuaPackRegister
{
public:
	void	beginPack(lua_State *L, const char *szName)
	{
		lua_newtable(L);
		m_nTableIndex = lua_gettop(L);
		lua_pushstring(L, szName);
		lua_pushvalue(L, m_nTableIndex);
		lua_settable(L, LUA_GLOBALSINDEX);
		m_pLua = L;
	}

	template <typename Func>
		void registerFunc(const char *szName, Func func)
	{
		lua_pushstring(m_pLua, szName);
		lua_pushlightuserdata(m_pLua, (void*)func);
		lua_pushcclosure(m_pLua, Lua2CPP_FuncDispatcher<Func>::Lua2CPP_FuncCall, 1);
		lua_settable(m_pLua, m_nTableIndex);
	}

	template <>
		void registerFunc(const char *szName, int (*fp)(lua_State*))
	{
		lua_pushstring(m_pLua, szName);
		lua_pushcfunction(m_pLua, fp);
		lua_settable(m_pLua, m_nTableIndex);
	}

	void	endPack()
	{
		m_nTableIndex = 0;
		lua_pop(m_pLua, 1);
		m_pLua = 0;
	}

	LuaPackRegister()
		:m_pLua(0), m_nTableIndex(0)
	{
	}

private:
	lua_State			*m_pLua;
	int					m_nTableIndex;
};
	

#endif //end __LUA_PACKAGE_H__