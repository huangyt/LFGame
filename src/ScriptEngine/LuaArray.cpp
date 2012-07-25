#include "LuaArray.h"

static const struct luaL_reg MyLuaArrayLib[] = 
{
	{ "new", LuaArray::pack_new },
	{ "size", LuaArray::pack_size },
	{ "set", LuaArray::proxy_set },
	{ "get", LuaArray::proxy_get },
	{ NULL, NULL }
};

void LuaArray::initialize(lua_State *L)
{
	if(L)
	{
		luaL_newmetatable(L, "MyArrayTable"); // meta
		luaL_register(L, "LuaArray", MyLuaArrayLib); // meta t

		lua_pushstring(L, "__index"); // meta t s
		lua_pushstring(L, "get"); // meta t s s1
		lua_gettable(L, -3);	// meta t s get
		lua_settable(L, -4);	// meta t

		lua_pushstring(L, "__newindex"); // meta t s
		lua_pushstring(L, "set"); // meta t s s1
		lua_gettable(L, -3);	// meta t s set
		lua_settable(L, -4);	// meta t

		lua_pop(L, 2);
	}
}

ArrayPrimNode* LuaArray::internal_check(lua_State *L, int idx)
{
	//! 首先检查传入的userdata是否具有MyArrayTable metatable
	void *ud = luaL_checkudata(L, idx, "MyArrayTable");
	//luaL_argcheck(L, ud != NULL, 1, "Invalid userdata!!");

	return (ArrayPrimNode*)ud;
}

ArrayPrimNode* LuaArray::internal_create(lua_State *L, int nNodes)
{
	nNodes = nNodes < 1 ? 1 : nNodes;
	int size = sizeof(ArrayPrimNode) * (nNodes + 1);
	ArrayPrimNode *pA = (ArrayPrimNode*)lua_newuserdata(L, size);
	pA->data.dl = nNodes;
	pA->type = LUA_TNIL;
	memset(&pA[1], 0, sizeof(ArrayPrimNode) * nNodes);

	luaL_getmetatable(L, "MyArrayTable");
	lua_setmetatable(L, -2);

	return pA; //! 注意，此时userdata在栈顶
}

void LuaArray::pushArray2Lua(lua_State *L, std::vector<ArrayPrimNode> &vec)
{
	if(L)
	{
		int nodes = vec.size();
		if(nodes > 0)
		{
			ArrayPrimNode *pA = internal_create(L, nodes);
			std::vector<ArrayPrimNode>::iterator it = vec.begin();
			memcpy(&pA[1], &vec[0], sizeof(ArrayPrimNode) * nodes);
		}
	}
}

bool LuaArray::userdata2Array(lua_State *L, int idx, std::vector<ArrayPrimNode> &vec)
{
	if(L)
	{
		ArrayPrimNode *pA = internal_check(L, idx);
		if(pA)
		{
			int size = static_cast<int>(pA->data.dl);
			vec.resize(size);
			std::vector<ArrayPrimNode>::iterator it = vec.begin();
			memcpy(&vec[0], &pA[1], sizeof(ArrayPrimNode) * size);
		}
	}

	return false;
}

int LuaArray::pack_size(lua_State *L)
{
	ArrayPrimNode *p = internal_check(L, 1);
	lua_pushnumber(L, p[0].data.dl);

	return 1;
}

int LuaArray::pack_new(lua_State *L)
{
	int nodes = luaL_checkint(L, 1);
	lua_pop(L, 1);
	internal_create(L, nodes);

	return 1;
}

int LuaArray::proxy_set(lua_State *L)
{
	ArrayPrimNode* p = internal_check(L, 1);

	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, 1 <= index && index <= p[0].data.dl, 2, "index out of range");

	int type = lua_type(L, 3);
	p[index].type = type;

	switch(type)
	{
	case LUA_TNUMBER:
		p[index].data.dl = lua_tonumber(L, 3);
		break;
	case LUA_TBOOLEAN:
		p[index].data.dl = lua_toboolean(L, 3);
		break;
	case LUA_TSTRING:
		p[index].data.ptr = (void*)lua_tostring(L, 3);
		break;
	case LUA_TLIGHTUSERDATA:
		p[index].data.ptr = lua_touserdata(L, 3);
		break;
		// unsupported type
		//case LUA_TUSERDATA:
		//case LUA_TTABLE:
		//case LUA_TFUNCTION:
		//case LUA_TTHREAD:
		//case LUA_TNIL:
	default:
		p[index].data.dl = 0;
		p[index].type = LUA_TNIL;
	}

	return 0;
}

int LuaArray::proxy_get(lua_State *L)
{
	ArrayPrimNode* p = internal_check(L, 1);

	int index = luaL_checkint(L, 2);
	luaL_argcheck(L, 1 <= index && index <= p[0].data.dl, 2, "index out of range");

	switch(p[index].type)
	{
	case LUA_TNUMBER:
		lua_pushnumber(L, p[index].data.dl);
		break;
	case LUA_TBOOLEAN:
		lua_pushboolean(L, static_cast<int>(p[index].data.dl));
		break;
	case LUA_TSTRING:
		lua_pushstring(L, (const char*)p[index].data.ptr);
		break;
	case LUA_TLIGHTUSERDATA:
		lua_pushlightuserdata(L, p[index].data.ptr);
		break;
		// unsupported type
		//case LUA_TUSERDATA:
		//case LUA_TTABLE:
		//case LUA_TFUNCTION:
		//case LUA_TTHREAD:
		//case LUA_TNIL:
	default:
		lua_pushnil(L);
		break;
	}

	return 1;
}