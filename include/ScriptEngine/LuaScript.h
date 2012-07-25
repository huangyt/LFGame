#ifndef __LUA_SCRIPT_H__
#define __LUA_SCRIPT_H__

#include "lua.hpp"
#include <string>

#define SCRIPT_LOAD_FAILED			-273341

//! lua脚本状态枚举
enum LUA_SCRIPT_STATE
{
	ELSS_EMPTY = 0,			   // 脚本为空，此时可执行新的chunk(当coroutine执行完毕，脚本状态也为空)
	//ELSS_SUSPENDED,			   // 等待状态，只要指chunk加载完成后
	ELSS_SUSPENDED_FRAMES,	   // 挂起N帧
	ELSS_SUSPENDED_SECONDS,	   // 挂起N秒
	ELSS_ERROR				   // 逻辑错误
};

class LuaScript;

//! lua脚本监听器
class LuaScriptListener
{
public:
	//! 通知接口
	virtual void			notify(LuaScript *p) = 0;

	//! 由LuaScript调用
	void					setFlag(bool b) { m_bPostFlag = b; }
	bool					getFlag(void) const { return m_bPostFlag; }

	LuaScriptListener()
		:m_bPostFlag(false)
	{
	}

	virtual ~LuaScriptListener() {}
private:
	bool					m_bPostFlag;
};

//! lua脚本类，一个脚本对象对应一个coroutine
// 注: 执行脚本方法doFile和doBuffer可临时绑定监听器，该监听器在coroutine执行
// 完毕时调用notify，之后将自动解除与LuaScript对象的绑定。 
class LuaScript
{
	friend class ScriptEngine;
public:
	union _Suspend
	{
		int			nFrames;
		float		fSeconds;
	};
public:
	LUA_SCRIPT_STATE	getState(void) const { return m_eState; }

	//! 由ScriptEngine调用
	void				update(float fElapsed);

	//! 由文件名执行脚本
	//  返回SCRIPT_LOAD_FAILED标识加载失败
	//  返回0表示执行完成，LUA_YIELD表示coroutine挂起，其他对应lua_resume error code
	int					doFile(const char *lua_fileName, LuaScriptListener *pListener = 0);

	//! 由直接的chunk数据执行脚本
	//  返回SCRIPT_LOAD_FAILED标识加载失败
	//  返回0表示执行完成，LUA_YIELD表示coroutine挂起，其他对应lua_resume error code
	int					doBuffer(const char *pBuf, size_t bytes, LuaScriptListener *pListener = 0);

	//! dump最后一次的错误描述
	size_t				dumpError(char *pError, size_t buffer_size);

	//! 获取最后一次捕获的用户描述
	const char*			getLastComment(void) const { return m_UserComment.c_str(); }

	//! resume the coroutine
	void				resume(void);

	//! 获取创建者
	ScriptEngine*		getCreator(void) { return m_pEngine; }

	//! 获取lua_State
	lua_State*			getLuaState(void) { return m_pluaThread; }

	//! 获取监听器
	LuaScriptListener*  getListener(void) const { return m_pListener; }

	//! 设置监听器
	//! 注：监听器只在执行脚本代码doFile、doBuffer时绑定到corountine，该方法主要用于清空
	//		监听器或者极特殊的场合中改变监听器
	void				setListener(LuaScriptListener *p);
private:
	int					resumeThread(float fParam);

	//! for ScriptEngine dispatch
	void				_SuspendScript(LUA_SCRIPT_STATE, _Suspend);
	void				_Return(LUA_SCRIPT_STATE, const char *);

	LuaScript(ScriptEngine *pEngine);
	~LuaScript();

private:
	LUA_SCRIPT_STATE	m_eState;
	lua_State			*m_pluaThread;
	_Suspend			m_Suspend;
	LuaScriptListener	*m_pListener;
	ScriptEngine		*m_pEngine;
	std::string			m_UserComment;
	bool				m_bTrashFlag;	//< special flag for engine
};

#endif //end __LUA_SCRIPT_H__