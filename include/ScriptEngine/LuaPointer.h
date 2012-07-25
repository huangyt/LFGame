#ifndef __LUA_POINTER_H__
#define __LUA_POINTER_H__

#include <lua.hpp>
#include <typeinfo.h>

class LuaPointer
{
public:
	struct sTrait
	{
		unsigned int t_v1;
		unsigned int t_v2;
		unsigned int t_v3;
	};
public:
	//! 需要在lua全局环境中初始化
	static void					initialize(lua_State *L);

	template <typename T>
		static void			pushOnToStack(lua_State *L, T *p)
	{
		if(L && p)
		{
			sTrait t;
			_get_trait(t, typeid(T).name());
			_push(L, t, (void*)p);
		}
	}

	template <typename T>
		static T*				checkPointer(lua_State *L, int idx)
	{
		if(L)
		{
			sTrait t;
			_get_trait(t, typeid(T).name());
			void *p = _check(L, t, idx);
			if(p)
			{
				return static_cast<T*>(p);
			}
		}

		return 0;
	}

	// for lua, check if the userdata is a LuaPointer
	static int				isValidPointer(lua_State *L);

private:
	static void				_get_trait(sTrait&, const char*);

	// create & push onto stack
	static void				_push(lua_State *L, const sTrait &T, void* p);

	static void*			_check(lua_State *L, const sTrait &T, int idx);				
};

#endif //end __LUA_POINTER_H__