#include "ScriptEngine.h"
#include "LuaConfig.h"
#include "LuaPackage.h"
#include "LuaArray.h"
#include "LuaPointer.h"
#include <algorithm>
#include <iostream>

void ScriptProfileDumper::dump(const char *str, bool bConole)
{
	if(bConole)
		std::cout<<str<<std::endl;

	dump_ex(str);
}

ScriptEngine::ScriptEngine()
{
#ifdef _LUA_CUSTOM_ALLOCATOR
	m_pLua = lua_newstate(L_ALL0C, NULL);
#else
	m_pLua = luaL_newstate();
#endif

	//! 打开所有标准库（changes in 5.1，所有单独的标准库打开需要lua_call形式）
	luaL_openlibs(m_pLua);

	LuaPackRegister rg;
	rg.beginPack(m_pLua, "ScriptEngine");
	rg.registerFunc("wait_seconds", ScriptEngine::suspendScript_Seconds);
	rg.registerFunc("wait_frames", ScriptEngine::suspendScript_Frame);
	//注 导出名为exit，为了和lua自带的lua function中的return进行区别
	rg.registerFunc("exit_s", ScriptEngine::return_s);
	rg.registerFunc("exit_error", ScriptEngine::return_error);
	rg.endPack();
	LuaArray::initialize(m_pLua);
	LuaPointer::initialize(m_pLua);
}

ScriptEngine::~ScriptEngine()
{
	destroyAllScripts();
	lua_close(m_pLua);
}

LuaScript* ScriptEngine::LtoLuaScript(lua_State *L)
{
	lua_pushlightuserdata(L, (void*)L);
	lua_gettable(L, LUA_GLOBALSINDEX);	// v 替代了 k
	LuaScript *pS = (LuaScript*)lua_touserdata(L, -1);
	lua_pop(L, 1);	// 弹出 v
	return pS;
}

int ScriptEngine::suspendScript_Frame(lua_State *L)
{
	LuaScript *p = LtoLuaScript(L);
	if(p)
	{
		LuaScript::_Suspend sus;
		sus.nFrames = (int)luaL_checknumber(L, 1);
		p->_SuspendScript(ELSS_SUSPENDED_FRAMES, sus);

		return lua_yield(L, 1);
	}

	return 1; //FIXME INVALID ERROR CODE
}

int ScriptEngine::suspendScript_Seconds(lua_State *L)
{
	LuaScript *p = LtoLuaScript(L);
	if(p)
	{
		LuaScript::_Suspend sus;
		sus.fSeconds = (float)luaL_checknumber(L, 1);
		p->_SuspendScript(ELSS_SUSPENDED_SECONDS, sus);

		return lua_yield(L, 1);
	}

	return 1; //FIXME INVALID ERROR CODE
}

int ScriptEngine::return_error(lua_State *L)
{
	LuaScript *p = LtoLuaScript(L);
	if(p)
	{
		const char* str = lua_tostring(L, 1);
		p->_Return(ELSS_ERROR, str);

		return lua_yield(L, 1);
	}

	return 1;
}

int ScriptEngine::return_s(lua_State *L)
{
	LuaScript *p = LtoLuaScript(L);
	if(p)
	{
		const char* str = lua_tostring(L, 1);
		p->_Return(ELSS_EMPTY, str);

		return lua_yield(L, 1);
	}

	return 1;
}

LuaScript* ScriptEngine::createScript()
{
	LuaScript *pS = new LuaScript(this);
	m_Scripts.push_back(pS);

	return pS;
}

void ScriptEngine::discardScript(LuaScript *p)
{
	if(p && p->getCreator() == this)
	{
		p->m_bTrashFlag = true;
	}
}

void ScriptEngine::destroyAllScripts()
{
	for(std::vector<LuaScript*>::iterator it = m_Scripts.begin();
		it != m_Scripts.end();it++)
	{
		delete (*it);
	}
	m_Scripts.clear();
}

void ScriptEngine::update(float fElapsed)
{
	for(std::vector<LuaScript*>::iterator it = m_Scripts.begin();
		it != m_Scripts.end();)
	{
		if((*it)->m_bTrashFlag)
		{
			LuaScript *p = (*it);
			delete p;
			it = m_Scripts.erase(it);
		}
		else
		{
			(*it)->update(fElapsed);
			it++;
		}
	}
}

ScriptProfileDumper* ScriptEngine::setLuaScriptDumper(ScriptProfileDumper *p)
{
	ScriptProfileDumper *pPre = 0;
	if(m_pDumper != p)
	{
		pPre = m_pDumper;
		m_pDumper = p;
	}

	return pPre;
}

void ScriptEngine::debug(const char *str, bool bOutPutConsole)
{
	if(!str)
		return;

	ScriptProfileDumper *pDumper = m_pDumper;
	static ScriptProfileDumper default_dumper;
	pDumper = pDumper ? pDumper : &default_dumper;

	char tmp[512] = {0};
	if(!strcmp("__STACK__", str))
	{
		pDumper->dump("Dumping lua global stack ...", bOutPutConsole);

		lua_pushnil(m_pLua);
		while(lua_next(m_pLua, LUA_GLOBALSINDEX) != 0)
		{
			sprintf(tmp, "<%s> - %s", lua_typename(m_pLua, lua_type(m_pLua, -1))
				, lua_tostring(m_pLua, -2));

			pDumper->dump(tmp, bOutPutConsole);
			lua_pop(m_pLua, 1);
		}
	}
	else
	{
		lua_getglobal(m_pLua, str);
		if(lua_istable(m_pLua, -1))
		{
			sprintf(tmp, "Dumping table %s ...", str);
			pDumper->dump(tmp, bOutPutConsole);
			lua_pushnil(m_pLua);
			while(lua_next(m_pLua, -2) != 0)
			{
				sprintf(tmp, "<%s> - %s", lua_typename(m_pLua, lua_type(m_pLua, -1))
					, lua_tostring(m_pLua, -2));

				pDumper->dump(tmp, bOutPutConsole);
				lua_pop(m_pLua, 1);
			}
		}			
	}
}
