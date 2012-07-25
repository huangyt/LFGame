#ifndef __SCRIPT_ENGINE_H__
#define __SCRIPT_ENGINE_H__

#include "LuaScript.h"
#include <vector>

// default dumper, support console dumping
class ScriptProfileDumper
{
public:
	virtual void		dump(const char *str, bool bConole = true);

protected:
	virtual void		dump_ex(const char *str) {}
};

class ScriptEngine
{
	friend class ScriptEngineFactory;
public:
	lua_State*			 getLuaState(void) const { return m_pLua; }

	LuaScript*			 createScript(void);

	//! ����ýű�Э���ѱ����ã����ڲ��ú󱻻��գ�
	void				 discardScript(LuaScript *p);

	void				 destroyAllScripts(void);

	void				 update(float fElapsed);

	ScriptProfileDumper* setLuaScriptDumper(ScriptProfileDumper *p);

	ScriptProfileDumper* getLuaScriptDumper(ScriptProfileDumper *p) const { return m_pDumper; }

	void				 debug(const char *str, bool bOutputConsole = true);

	ScriptEngine();
	~ScriptEngine();

	//! ע�ᵽlua���ⲿ���ɵ���
	static int			 suspendScript_Frame(lua_State *L);
	static int		     suspendScript_Seconds(lua_State *L);
	static int			 return_error(lua_State *L);
	static int			 return_s(lua_State *L);
private:
	//void				 initialize(ScriptEngineInitializer& initzer);
	static LuaScript*	 LtoLuaScript(lua_State *L);

	ScriptEngine(const ScriptEngine &eng);
	ScriptEngine& operator = (const ScriptEngine &eng);
private:
	lua_State					*m_pLua;
	ScriptProfileDumper			*m_pDumper;
	std::vector<LuaScript*>		m_Scripts;
};

#endif //end __SCRIPT_ENGINE_H__