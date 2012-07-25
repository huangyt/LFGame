#ifndef __LUA_PROXY_H__
#define __LUA_PROXY_H__

#include "lua.hpp"

inline bool Lua2CPP_TypeMatch(lua_State *L, int idx, bool&)
{
	luaL_checktype(L, idx, LUA_TBOOLEAN);
	return 1 == lua_toboolean(L, idx);
}

inline char Lua2CPP_TypeMatch(lua_State *L, int idx, char&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<char>(lua_tonumber(L, idx));
}

inline unsigned char Lua2CPP_TypeMatch(lua_State *L, int idx, unsigned char&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<unsigned char>(lua_tonumber(L, idx));
}

inline short Lua2CPP_TypeMatch(lua_State *L, int idx, short&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<short>(lua_tonumber(L, idx));
}

inline unsigned short Lua2CPP_TypeMatch(lua_State *L, int idx, unsigned short&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<unsigned short>(lua_tonumber(L, idx));
}

inline int Lua2CPP_TypeMatch(lua_State *L, int idx, int&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return lua_tointeger(L, idx);
}

inline unsigned int Lua2CPP_TypeMatch(lua_State *L, int idx, unsigned int&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<unsigned int>(lua_tonumber(L, idx));
}

inline long Lua2CPP_TypeMatch(lua_State *L, int idx, long&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<long>(lua_tonumber(L, idx));
}

inline unsigned long Lua2CPP_TypeMatch(lua_State *L, int idx, unsigned long&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<unsigned long>(lua_tonumber(L, idx));
}

inline float Lua2CPP_TypeMatch(lua_State *L, int idx, float&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return static_cast<float>(lua_tonumber(L, idx));
}

inline double Lua2CPP_TypeMatch(lua_State *L, int idx, double&)
{
	luaL_checktype(L, idx, LUA_TNUMBER);
	return lua_tonumber(L, idx);
}

inline const char* Lua2CPP_TypeMatch(lua_State *L, int idx, const char*&)
{
	luaL_checktype(L, idx, LUA_TSTRING);
	return lua_tostring(L, idx);
}

inline void* Lua2CPP_TypeMatch(lua_State *L, int idx, void*&)
{
	luaL_checktype(L, idx, LUA_TLIGHTUSERDATA);
	return lua_touserdata(L, idx);
}

inline void CPP2Lua_Push(lua_State *L, bool b)
{
	lua_pushboolean(L, static_cast<int>(b));
}

inline void CPP2Lua_Push(lua_State *L, char c)
{
	lua_pushinteger(L, static_cast<int>(c));
}

inline void CPP2Lua_Push(lua_State *L, unsigned char c)
{
	lua_pushinteger(L, static_cast<int>(c));
}

inline void CPP2Lua_Push(lua_State *L, short n)
{
	lua_pushinteger(L, n);
}

inline void CPP2Lua_Push(lua_State *L, unsigned short n)
{
	lua_pushinteger(L, n);
}

inline void CPP2Lua_Push(lua_State *L, int n)
{
	lua_pushinteger(L, n);
}

inline void CPP2Lua_Push(lua_State *L, unsigned int n)
{
	lua_pushnumber(L, n);
}

inline void CPP2Lua_Push(lua_State *L, long l)
{
	lua_pushnumber(L, l);
}

inline void CPP2Lua_Push(lua_State *L, unsigned long l)
{
	lua_pushnumber(L, l);
}

inline void CPP2Lua_Push(lua_State *L, float f)
{
	lua_pushnumber(L, f);
}

inline void CPP2Lua_Push(lua_State *L, double d)
{
	lua_pushnumber(L, d);
}

inline void CPP2Lua_Push(lua_State *L, const char* s)
{
	lua_pushstring(L, s);
}

inline void CPP2Lua_Push(lua_State *L, void* p)
{
	lua_pushlightuserdata(L, p);
}

template <typename RT>
class Lua2CPP_Dispatcher
{
public:
	//////////////////////////////////////////////////////////////////////////
	// for export function
	//////////////////////////////////////////////////////////////////////////

	static int call(lua_State *L, RT (*fp)())
	{
		RT r = fp();
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1>
		static int call(lua_State *L, RT (*fp)(P1))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		RT r = fp(p1);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1, typename P2>
		static int call(lua_State *L, RT (*fp)(P1, P2))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		RT r = fp(p1, p2);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1, typename P2, typename P3>
		static int call(lua_State *L, RT (*fp)(P1, P2, P3))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		RT r = fp(p1, p2, p3);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1,typename P2, typename P3, typename P4>
		static int call(lua_State *L, RT (*fp)(P1, P2, P3, P4))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		RT r = fp(p1, p2, p3, p4);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
		static int call(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		RT r = fp(p1, p2, p3, p4, p5);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5,
		typename P6>
		static int call(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5, P6))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
		RT r = fp(p1, p2, p3, p4, p5, p6);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5,
		typename P6, typename P7>
		static int call(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5, P6, P7))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
		P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
		RT r = fp(p1, p2, p3, p4, p5, p6, p7);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5,
		typename P6, typename P7, typename P8>
		static int call(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5, P6, P7, P8))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
		P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
		P8 p8 = Lua2CPP_TypeMatch(L, 8, p8);
		RT r = fp(p1, p2, p3, p4, p5, p6, p7, p8);
		CPP2Lua_Push(L, r);
		return 1;
	}

	//////////////////////////////////////////////////////////////////////////
	// for export class member function
	//////////////////////////////////////////////////////////////////////////

	template <typename Class>
		static int call(lua_State *L, Class *p, RT (Class::*fp)())
	{
		RT r = (p->*fp)();
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		RT r = (p->*fp)(p1);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		RT r = (p->*fp)(p1, p2);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2, typename P3>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		RT r = (p->*fp)(p1, p2, p3);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2, typename P3, typename P4>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		RT r = (p->*fp)(p1, p2, p3, p4);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2, typename P3, typename P4,
		typename P5>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		RT r = (p->*fp)(p1, p2, p3, p4, p5);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2, typename P3, typename P4,
		typename P5, typename P6>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5, P6))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
		RT r = (p->*fp)(p1, p2, p3, p4, p5, p6);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2, typename P3, typename P4,
		typename P5, typename P6, typename P7>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5, P6, P7))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
		P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
		RT r = (p->*fp)(p1, p2, p3, p4, p5, p6, p7);
		CPP2Lua_Push(L, r);
		return 1;
	}

	template <typename Class, typename P1, typename P2, typename P3, typename P4,
		typename P5, typename P6, typename P7, typename P8>
		static int call(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5, P6, P7, P8))
	{
		P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
		P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
		P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
		P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
		P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
		P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
		P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
		P8 p8 = Lua2CPP_TypeMatch(L, 8, p8);
		RT r = (p->*fp)(p1, p2, p3, p4, p5, p6, p7, p8);
		CPP2Lua_Push(L, r);
		return 1;
	}

};

// 部分特化return void
template <>
	class Lua2CPP_Dispatcher<void>
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// for export function
		//////////////////////////////////////////////////////////////////////////

		static int call(lua_State *L, void (*fp)())
		{
			(void)L;
			fp();
			return 0;
		}

		template <typename P1>
			static int call(lua_State *L, void (*fp)(P1))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			fp(p1);
			return 0;
		}

		template <typename P1, typename P2>
			static int call(lua_State *L, void (*fp)(P1, P2))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			fp(p1, p2);
			return 0;
		}

		template <typename P1, typename P2, typename P3>
			static int call(lua_State *L, void (*fp)(P1, P2, P3))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			fp(p1, p2, p3);
			return 0;
		}

		template <typename P1,typename P2, typename P3, typename P4>
			static int call(lua_State *L, void (*fp)(P1, P2, P3, P4))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			fp(p1, p2, p3, p4);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
			static int call(lua_State *L, void (*fp)(P1, P2, P3, P4, P5))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			fp(p1, p2, p3, p4, p5);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5,
			typename P6>
			static int call(lua_State *L, void (*fp)(P1, P2, P3, P4, P5, P6))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
			fp(p1, p2, p3, p4, p5, p6);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5,
			typename P6, typename P7>
			static int call(lua_State *L, void (*fp)(P1, P2, P3, P4, P5, P6, P7))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
			P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
			fp(p1, p2, p3, p4, p5, p6, p7);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5,
			typename P6, typename P7, typename P8>
			static int call(lua_State *L, void (*fp)(P1, P2, P3, P4, P5, P6, P7, P8))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
			P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
			P8 p8 = Lua2CPP_TypeMatch(L, 8, p8);
			fp(p1, p2, p3, p4, p5, p6, p7, p8);
			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// for export class member function
		//////////////////////////////////////////////////////////////////////////

		template <typename Class>
			static int call(lua_State *L, Class *p, void (Class::*fp)())
		{
			(void)L;
			(p->*fp)();
			return 0;
		}

		template <typename Class, typename P1>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			(p->*fp)(p1);
			return 0;
		}

		template <typename Class, typename P1, typename P2>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			(p->*fp)(p1, p2);
			return 0;
		}

		template <typename Class, typename P1, typename P2, typename P3>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2, P3))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			(p->*fp)(p1, p2, p3);
			return 0;
		}

		template <typename Class, typename P1, typename P2, typename P3, typename P4>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2, P3, P4))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			(p->*fp)(p1, p2, p3, p4);
			return 0;
		}

		template <typename Class, typename P1, typename P2, typename P3, typename P4,
			typename P5>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2, P3, P4, P5))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			(p->*fp)(p1, p2, p3, p4, p5);
			return 0;
		}

		template <typename Class, typename P1, typename P2, typename P3, typename P4,
			typename P5, typename P6>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2, P3, P4, P5, P6))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
			(p->*fp)(p1, p2, p3, p4, p5, p6);
			return 0;
		}

		template <typename Class, typename P1, typename P2, typename P3, typename P4,
			typename P5, typename P6, typename P7>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2, P3, P4, P5, P6, P7))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
			P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
			(p->*fp)(p1, p2, p3, p4, p5, p6, p7);
			return 0;
		}

		template <typename Class, typename P1, typename P2, typename P3, typename P4,
			typename P5, typename P6, typename P7, typename P8>
			static int call(lua_State *L, Class *p, void (Class::*fp)(P1, P2, P3, P4, P5, P6, P7, P8))
		{
			P1 p1 = Lua2CPP_TypeMatch(L, 1, p1);
			P2 p2 = Lua2CPP_TypeMatch(L, 2, p2);
			P3 p3 = Lua2CPP_TypeMatch(L, 3, p3);
			P4 p4 = Lua2CPP_TypeMatch(L, 4, p4);
			P5 p5 = Lua2CPP_TypeMatch(L, 5, p5);
			P6 p6 = Lua2CPP_TypeMatch(L, 6, p6);
			P7 p7 = Lua2CPP_TypeMatch(L, 7, p7);
			P8 p8 = Lua2CPP_TypeMatch(L, 8, p8);
			(p->*fp)(p1, p2, p3, p4, p5, p6, p7, p8);
			return 0;
		}
	};

//////////////////////////////////////////////////////////////////////////
// return value type detacher
//////////////////////////////////////////////////////////////////////////

template <typename RT>
	int RetVal_Detach(lua_State *L, RT (*fp)())
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2, typename P3>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2, P3))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2, typename P3, typename P4>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2, P3, P4))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5, P6))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5, P6, P7))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7, typename P8>
	int RetVal_Detach(lua_State *L, RT (*fp)(P1, P2, P3, P4, P5, P6, P7, P8))
{
	return Lua2CPP_Dispatcher<RT>::call(L, fp);
}

template <typename Class, typename RT>
	int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)())
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2, typename P3>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2, typename P3, typename P4>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5, P6))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5, P6, P7))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}

template <typename Class, typename RT, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7, typename P8>
int RetVal_Detach(lua_State *L, Class *p, RT (Class::*fp)(P1, P2, P3, P4, P5, P6, P7, P8))
{
	return Lua2CPP_Dispatcher<RT>::call(L, p, fp);
}


//! 直接函数调用和注册
template <typename Func>
class Lua2CPP_FuncDispatcher
{
public:
	static int Lua2CPP_FuncCall(lua_State *L)
	{
		Func fp = static_cast<Func>(lua_touserdata(L, lua_upvalueindex(1)));

		return RetVal_Detach(L, fp);
	}
};

template <typename Func>
inline void CPP2Lua_RegisterFunc(lua_State *L, const char *name, Func func)
{
	lua_pushstring(L, name);
	lua_pushlightuserdata(L, (void*)func);
	lua_pushcclosure(L, Lua2CPP_FuncDispatcher<Func>::Lua2CPP_FuncCall, 1);
	lua_settable(L, LUA_GLOBALSINDEX);
}

//! 类成员注册由LuaClass辅助类进行耦合

#endif //end __LUA_PROXY_H__