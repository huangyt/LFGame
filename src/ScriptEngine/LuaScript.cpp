#include "LuaScript.h"
#include "ScriptEngine.h"
#include <string.h>

#define SCRIPT_COROUTINE_FINISHED			0


LuaScript::LuaScript(ScriptEngine *pEngine)
	:m_eState(ELSS_EMPTY), m_pListener(0), m_pEngine(pEngine), m_bTrashFlag(false)
{
	m_Suspend.nFrames = 0;
	// push thread to table for refereance
	lua_State *pEngine_luaState = m_pEngine->getLuaState();
	lua_pushlightuserdata(pEngine_luaState, this);
	m_pluaThread = lua_newthread(pEngine_luaState);
	lua_settable(pEngine_luaState, LUA_GLOBALSINDEX);

	// push this to table for converting
	lua_pushlightuserdata(pEngine_luaState, m_pluaThread);
	lua_pushlightuserdata(pEngine_luaState, this);
	lua_settable(pEngine_luaState, LUA_GLOBALSINDEX);
}

LuaScript::~LuaScript()
{
	lua_State *g_L = m_pEngine->getLuaState();

	lua_pushlightuserdata(g_L, m_pluaThread);
	lua_pushnil(g_L);
	lua_settable(g_L, LUA_GLOBALSINDEX);

	lua_pushlightuserdata(g_L, this);
	lua_pushnil(g_L);
	lua_settable(g_L, LUA_GLOBALSINDEX);
}

void LuaScript::setListener(LuaScriptListener *p)
{
	m_pListener = p;
}

void LuaScript::resume()
{
	if(m_eState == ELSS_SUSPENDED_FRAMES || m_eState == ELSS_SUSPENDED_SECONDS)
	{
		resumeThread(0);
	}
}

int LuaScript::resumeThread(float fParam)
{
	lua_pushnumber(m_pluaThread, (lua_Number)fParam);
	int stat = lua_resume(m_pluaThread, 1);
	
	if(stat == LUA_YIELD)
	{
		stat = m_eState == ELSS_EMPTY ? 0 : LUA_YIELD;
	}
	else
	{
		if(!stat)
		{
			m_eState = ELSS_EMPTY;
		}
		else
		{
			m_eState = ELSS_ERROR;
		}
	}

	return stat;
}

int LuaScript::doFile(const char *lua_fileName, LuaScriptListener *pListener)
{
	int stat = -1;
	if(lua_fileName)
	{
		stat = luaL_loadfile(m_pluaThread, lua_fileName);
		if(!stat)
		{
			stat = resumeThread(0);
			if(pListener)
			{
				if(m_eState == ELSS_SUSPENDED_FRAMES
					|| m_eState == ELSS_SUSPENDED_SECONDS)
				{
					m_pListener = pListener;
				}
				else
				{
					pListener->notify(this);
				}
			}
		}
		else
		{
			return SCRIPT_LOAD_FAILED;
		}
	}

	return stat;
}

int LuaScript::doBuffer(const char *pBuf, size_t bytes, LuaScriptListener *pListener)
{
	int stat = -1;
	if(pBuf)
	{
		stat = luaL_loadbuffer(m_pluaThread, pBuf, bytes, 0);
		if(!stat)
		{
			stat = resumeThread(0);
			if(pListener)
			{
				if(m_eState == ELSS_SUSPENDED_FRAMES
					|| m_eState == ELSS_SUSPENDED_SECONDS)
				{
					m_pListener = pListener;
				}
				else
				{
					pListener->notify(this);
				}
			}
		}
		else
		{
			return SCRIPT_LOAD_FAILED;
		}
	}

	return stat;
}

void LuaScript::_SuspendScript(LUA_SCRIPT_STATE e, _Suspend sus)
{
	if(e == ELSS_SUSPENDED_FRAMES)
	{
		m_Suspend.nFrames = sus.nFrames;
		m_eState = ELSS_SUSPENDED_FRAMES;
	}
	else if(e == ELSS_SUSPENDED_SECONDS)
	{
		m_eState = ELSS_SUSPENDED_SECONDS;
		m_Suspend.fSeconds = sus.fSeconds;
	}
}

void LuaScript::_Return(LUA_SCRIPT_STATE e, const char *szComment)
{
	m_eState = e;
	if(szComment)
	{
		m_UserComment = szComment;
	}

	if(m_pListener)
	{
		m_pListener->setFlag(true);
	}
}

void LuaScript::update(float fElapsed)
{
	if(m_pListener && m_pListener->getFlag())
	{
		m_pListener->setFlag(false);
		m_pListener->notify(this);
		m_pListener = 0;
	}

	switch(m_eState)
	{
	case ELSS_SUSPENDED_FRAMES:
		{
			// <= 0 means suspends forerver
			if(m_Suspend.nFrames > 0)
			{
				if(--m_Suspend.nFrames <= 0)
				{
					resumeThread(0);
				}
			}
		}
		break;
	case ELSS_SUSPENDED_SECONDS:
		{
			// <= 0 means suspends forever
			if(m_Suspend.fSeconds > 0)
			{
				m_Suspend.fSeconds -= fElapsed;
				if(m_Suspend.fSeconds <= 0)
				{
					m_Suspend.fSeconds = 0;
					resumeThread(0);
				}
			}
		}
		break;
	}

	//! 如果脚本中出现非交互式return
	if(m_pListener)
	{
		if(m_eState == ELSS_EMPTY || m_eState == ELSS_ERROR)
		{
			m_pListener->notify(this);
			m_pListener = 0;
		}
	}
}

size_t LuaScript::dumpError(char *pError, size_t buffer_size)
{
	if(pError && buffer_size > 0)
	{
		const char *error_msg = lua_tostring(m_pluaThread, -1);
		if(error_msg)
		{
			size_t nLen = strlen(error_msg);
			buffer_size = nLen > buffer_size - 1 ? buffer_size - 1 : nLen;
			strncpy(pError, error_msg, buffer_size);
			return buffer_size;
		}
	}

	return 0;
}