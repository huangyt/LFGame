#include "LuaPointer.h"

struct sMyLuaPointer
{
	unsigned int n1;
	unsigned int n2;
	unsigned int n3;
	void *ptr;
};

unsigned int elf(const char *str) 
{ 
	unsigned int hash = 0; 
	unsigned int x = 0; 
	const char *p = str;

	while(*p) 
	{ 
		hash = (hash << 4) + (*p++); 
		if ((x = hash & 0xF0000000L ) != 0) 
		{ 
			hash ^= (x >> 24); 
			hash &= ~ x; 
		} 
	} 

	return (hash & 0x7FFFFFFF); 
} 

unsigned int fnv(const char *str) 
{ 
	const char *p = str; 
	unsigned int hash = 0; 

	while(*p)
	{ 
		hash *= 16777619; 
		hash ^= (unsigned int)((unsigned char)(*p++)); 
	}

	return hash; 
}


unsigned int sim(const char *str) 
{ 
	unsigned int h; 
	const unsigned char *p; 

	for(h=0, p = (const unsigned char*)str; *p; p++) 
		h = 31 * h + *p; 

	return h; 
}

bool lpt_equal(const LuaPointer::sTrait &n1, const LuaPointer::sTrait &n2)
{
	return n1.t_v1 == n2.t_v1 && n1.t_v2 == n2.t_v2
		&& n1.t_v3 == n2.t_v3;
}

bool lpt_equal(const LuaPointer::sTrait &n, const sMyLuaPointer &pt)
{
	return n.t_v1 == pt.n1 && n.t_v2 == pt.n2 && n.t_v3 == pt.n3;
}

static const struct luaL_reg MyLuaPointerLib[] = 
{
	{ "is_valid", LuaPointer::isValidPointer },
	{ NULL, NULL }
};

void LuaPointer::initialize(lua_State *L)
{
	if(L)
	{
		luaL_newmetatable(L, "MyLuaPointerTable"); // meta
		luaL_register(L, "LuaPointer", MyLuaPointerLib); // meta t

		lua_pop(L, 2);
	}
}

int LuaPointer::isValidPointer(lua_State *L)
{
	void* p = luaL_checkudata(L, 1, "MyLuaPointerTable");
	lua_pushboolean(L, p != 0 ? 1 : 0);

	return 1;
}

void LuaPointer::_get_trait(sTrait& T, const char* s)
{
	T.t_v1 = elf(s);
	T.t_v2 = fnv(s);
	T.t_v3 = sim(s);
}

void* LuaPointer::_check(lua_State *L, const LuaPointer::sTrait &T, int idx)
{
	void* p = luaL_checkudata(L, idx, "MyLuaPointerTable");
	if(p)
	{
		sMyLuaPointer *pp = (sMyLuaPointer*)p;
		if(lpt_equal(T, *pp))
		{
			return pp->ptr;
		}
	}

	return 0;
}

void LuaPointer::_push(lua_State *L, const LuaPointer::sTrait &T, void* p)
{
	sMyLuaPointer *ptr = (sMyLuaPointer*)lua_newuserdata(L, sizeof(sMyLuaPointer));
	ptr->n1 = T.t_v1;
	ptr->n2 = T.t_v2;
	ptr->n3 = T.t_v3;
	ptr->ptr = p;
	luaL_getmetatable(L, "MyLuaPointerTable");
	lua_setmetatable(L, -2);
}