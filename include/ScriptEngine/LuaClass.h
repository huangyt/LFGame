#ifndef __LUA_CLASS_H__
#define __LUA_CLASS_H__

#include "LuaProxy.h"

//! 向lua种导出类的模板
template <typename T>
class LuaClass
{
	// user data type
	struct userdataType
	{
		T		*ptr;
	};
public:
	typedef int (T::*ExtensionMemberFunc)(lua_State *L);

	// 必须以register_begin(...) >> register_mfunc(...) >> register_end(...)风格撰写代码
	static void register_begin(lua_State *L)
	{
		lua_newtable(L);
		int methods = lua_gettop(L);

		luaL_newmetatable(L, className);
		int metatable = lua_gettop(L);

		lua_pushvalue(L, methods);
		set(L, LUA_GLOBALSINDEX, className);

		// hide metatable from Lua getmetatable()
		lua_pushvalue(L, methods);
		set(L, metatable, "__metatable");

		lua_pushvalue(L, methods);
		set(L, metatable, "__index");

		lua_pushcfunction(L, tostring_T);
		set(L, metatable, "__tostring");

		lua_pushcfunction(L, gc_T);
		set(L, metatable, "__gc");

		lua_newtable(L);                // mt for method table
		lua_pushcfunction(L, new_T);
		lua_pushvalue(L, -1);           // dup new_T function
		set(L, methods, "new");         // add new_T to method table
		set(L, -3, "__call");           // mt.__call = new_T
		lua_setmetatable(L, methods);

		s_nMethodTable = methods;
		s_pLua = L;
	}

	// register the member function
	template <typename Func>
		static void register_mfunc(const char *name, Func func)
	{
		lua_pushstring(s_pLua, name);
		void *p = lua_newuserdata(s_pLua, sizeof(func));
		memcpy(p, &func, sizeof(func));
		lua_pushcclosure(s_pLua, thunk<Func>, 1);
		lua_settable(s_pLua, s_nMethodTable);
	}

	// T::*ExtensionMemberFunc
	template <>
		static void register_mfunc(const char *name, int (T::*fp)(lua_State*))
	{
		lua_pushstring(s_pLua, name);
		void *p = lua_newuserdata(s_pLua, sizeof(fp));
		memcpy(p, &fp, sizeof(fp));
		lua_pushcclosure(s_pLua, thunk_ex, 1);
		lua_settable(s_pLua, s_nMethodTable);		
	}

	// end register
	static void register_end()
	{
		lua_pop(s_pLua, 2);
	}

	// call named lua method from userdata method table
	static int call(lua_State *L, const char *method,
		int nargs = 0, int nresults = LUA_MULTRET, int errfunc = 0)
	{
		int base = lua_gettop(L) - nargs;
		if(!luaL_checkudata(L, base, className))
		{
			lua_settop(L, base-1);
			lua_pushfstring(L, "not a valid %s userdata", className);
			return -1;
		}

		lua_pushstring(L, method);
		lua_gettable(L, base);
		if(lua_isnil(L, -1))
		{ // no method?
			lua_settop(L, base-1);
			lua_pushfstring(L, "Undefined %s method '%s'", className, method);
			return -1;
		}
		lua_insert(L, base);

		int status = lua_pcall(L, 1+nargs, nresults, errfunc);
		if(status)
		{
			const char *msg = lua_tostring(L, -1);
			if (msg == NULL) msg = "(unknown error)";
			lua_pushfstring(L, "%s:%s status = %d\n%s",
				className, method, status, msg);
			lua_remove(L, base);
			return -1;
		}
		return lua_gettop(L) - base + 1;   // number of results
	}

	// push a lightuserdata (pointer)
	static int push(lua_State *L, T *obj, bool gc = false)
	{
		if(!obj)
		{ 
			lua_pushnil(L);
			return 0;
		}

		luaL_getmetatable(L, className);  // lookup metatable in Lua registry
		if(lua_isnil(L, -1))
		{
			luaL_error(L, "%s missing metatable", className);
		}

		int mt = lua_gettop(L);
		subtable(L, mt, "userdata", "v");
		userdataType *ud = static_cast<userdataType*>(pushuserdata(L, obj, sizeof(userdataType)));
		if(ud)
		{
			ud->ptr = obj;  // store pointer to object in userdata
			lua_pushvalue(L, mt);
			lua_setmetatable(L, -2);
			if(!gc)
			{
				lua_checkstack(L, 3);
				subtable(L, mt, "do not trash", "k");
				lua_pushvalue(L, -2);
				lua_pushboolean(L, 1);
				lua_settable(L, -3);
				lua_pop(L, 1);
			}
		}
		lua_replace(L, mt);
		lua_settop(L, mt);
		return mt;  // index of userdata containing pointer to T object
	}

	// get lightuserdata (pointer)
	static T* check(lua_State *L, int narg)
	{
		userdataType *ud = static_cast<userdataType*>(luaL_checkudata(L, narg, className));
		if(!ud)
		{
			luaL_typerror(L, narg, className);
			return NULL;
		}
		return ud->ptr;
	}

private:

	// member function dispatcher
	template <typename Func>
		static int thunk(lua_State *L)
	{
		T *obj = check(L, 1);
		lua_remove(L, 1);

		Func fp = *static_cast<Func*>(lua_touserdata(L, lua_upvalueindex(1)));

		return RetVal_Detach(L, obj, fp);
	}

	// member function dispatcher, extension version
	static int thunk_ex(lua_State *L)
	{
		T *obj = check(L, 1);
		lua_remove(L, 1);

		//MFuncReg *l = static_cast<MFuncReg*>(lua_touserdata(L, lua_upvalueindex(1)));
		//return (obj->*(l->mfunc))(L);

		ExtensionMemberFunc fp = *static_cast<ExtensionMemberFunc*>(lua_touserdata(L, lua_upvalueindex(1)));

		return (obj->*(fp))(L);
	}

	// __call
	static int new_T(lua_State *L)
	{
		lua_remove(L, 1);
		T *obj = new T(L);		//! 传入此参数意味着导出类必须实现以lua_State为参数的构造函数
		push(L, obj, true);
		return 1;
	}

	// __gc
	static int gc_T(lua_State *L)
	{
		if(luaL_getmetafield(L, 1, "do not trash"))
		{
			lua_pushvalue(L, 1);
			lua_gettable(L, -2);
			if (!lua_isnil(L, -1)) return 0;
		}
		userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
		T *obj = ud->ptr;
		if(obj)
			delete obj;

		return 0;
	}

	// __tostring
	static int tostring_T (lua_State *L)
	{
		char buff[32];
		userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
		T *obj = ud->ptr;
		sprintf_s(buff,32, "%p", (void*)obj);
		lua_pushfstring(L, "%s (%s)", className, buff);

		return 1;
	}

	// set the table (key - balue method)
	static void set(lua_State *L, int table_index, const char *key)
	{
		lua_pushstring(L, key);
		lua_insert(L, -2);
		lua_settable(L, table_index);
	}

	// __mode = \"k\" \"v\"
	static void weaktable(lua_State *L, const char *mode)
	{
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setmetatable(L, -2);
		lua_pushliteral(L, "__mode");
		lua_pushstring(L, mode);
		lua_settable(L, -3);   // metatable.__mode = mode
	}

	static void subtable(lua_State *L, int tindex, const char *name, const char *mode)
	{
		lua_pushstring(L, name);
		lua_gettable(L, tindex);
		if(lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			lua_checkstack(L, 3);
			weaktable(L, mode);
			lua_pushstring(L, name);
			lua_pushvalue(L, -2);
			lua_settable(L, tindex);
		}
	}

	static void* pushuserdata(lua_State *L, void *key, size_t sz)
	{
		void *ud = 0;
		lua_pushlightuserdata(L, key);
		lua_gettable(L, -2);
		if(lua_isnil(L, -1))
		{
			lua_pop(L, 1);
			lua_checkstack(L, 3);
			ud = lua_newuserdata(L, sz);
			lua_pushlightuserdata(L, key);
			lua_pushvalue(L, -2);
			lua_settable(L, -4);
		}
		return ud;
	}

	LuaClass()
	{
	}

public:
	static const char className[];
	static lua_State *s_pLua;
	static int		  s_nMethodTable;
};

#define LUA_METHOD(Func) int Func(lua_State*)
#define LUA_CALSS_TEMPLATE_IMPLEMENT(Class) const char LuaClass<Class>::className[] = #Class;\
	lua_State* LuaClass<Class>::s_pLua = 0;\
	int LuaClass<Class>::s_nMethodTable = 0;

#endif //end __LUA_CLASS_H__