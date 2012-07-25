#ifndef __LUA_SCRIPT_H__
#define __LUA_SCRIPT_H__

#include "lua.hpp"
#include <string>

#define SCRIPT_LOAD_FAILED			-273341

//! lua�ű�״̬ö��
enum LUA_SCRIPT_STATE
{
	ELSS_EMPTY = 0,			   // �ű�Ϊ�գ���ʱ��ִ���µ�chunk(��coroutineִ����ϣ��ű�״̬ҲΪ��)
	//ELSS_SUSPENDED,			   // �ȴ�״̬��ֻҪָchunk������ɺ�
	ELSS_SUSPENDED_FRAMES,	   // ����N֡
	ELSS_SUSPENDED_SECONDS,	   // ����N��
	ELSS_ERROR				   // �߼�����
};

class LuaScript;

//! lua�ű�������
class LuaScriptListener
{
public:
	//! ֪ͨ�ӿ�
	virtual void			notify(LuaScript *p) = 0;

	//! ��LuaScript����
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

//! lua�ű��࣬һ���ű������Ӧһ��coroutine
// ע: ִ�нű�����doFile��doBuffer����ʱ�󶨼��������ü�������coroutineִ��
// ���ʱ����notify��֮���Զ������LuaScript����İ󶨡� 
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

	//! ��ScriptEngine����
	void				update(float fElapsed);

	//! ���ļ���ִ�нű�
	//  ����SCRIPT_LOAD_FAILED��ʶ����ʧ��
	//  ����0��ʾִ����ɣ�LUA_YIELD��ʾcoroutine����������Ӧlua_resume error code
	int					doFile(const char *lua_fileName, LuaScriptListener *pListener = 0);

	//! ��ֱ�ӵ�chunk����ִ�нű�
	//  ����SCRIPT_LOAD_FAILED��ʶ����ʧ��
	//  ����0��ʾִ����ɣ�LUA_YIELD��ʾcoroutine����������Ӧlua_resume error code
	int					doBuffer(const char *pBuf, size_t bytes, LuaScriptListener *pListener = 0);

	//! dump���һ�εĴ�������
	size_t				dumpError(char *pError, size_t buffer_size);

	//! ��ȡ���һ�β�����û�����
	const char*			getLastComment(void) const { return m_UserComment.c_str(); }

	//! resume the coroutine
	void				resume(void);

	//! ��ȡ������
	ScriptEngine*		getCreator(void) { return m_pEngine; }

	//! ��ȡlua_State
	lua_State*			getLuaState(void) { return m_pluaThread; }

	//! ��ȡ������
	LuaScriptListener*  getListener(void) const { return m_pListener; }

	//! ���ü�����
	//! ע��������ֻ��ִ�нű�����doFile��doBufferʱ�󶨵�corountine���÷�����Ҫ�������
	//		���������߼�����ĳ����иı������
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