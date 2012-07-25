#ifndef __LUA_ARRAY_H__
#define __LUA_ARRAY_H__

#include "lua.hpp"
#include <vector>

struct ArrayPrimNode
{
	int						type;
	union _data
	{
		double				dl;
		void*				ptr;
	};
	_data					data;
};

//////////////////////////////////////////////////////////////////////////
// 由C++类型获取对应的Lua类型

inline int ToLuaType(bool b)
{
	return LUA_TBOOLEAN;
}

inline int ToLuaType(char c)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(unsigned char c)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(short s)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(unsigned short s)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(int n)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(unsigned int n)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(long l)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(unsigned long l)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(float f)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(double d)
{
	return LUA_TNUMBER;
}

inline int ToLuaType(const char *str)
{
	return LUA_TSTRING;
}

inline int ToLuaType(void *p)
{
	return LUA_TLIGHTUSERDATA;
}

//////////////////////////////////////////////////////////////////////////
// 将C++类型数据写入ArrayPrimNode

inline void ToArrayPrimType(bool b, ArrayPrimNode &node)
{
	node.type = ToLuaType(b);
	node.data.dl = b;
}

inline void ToArrayPrimType(char c, ArrayPrimNode &node)
{
	node.type = ToLuaType(c);
	node.data.dl = c;
}

inline void ToArrayPrimType(unsigned char c, ArrayPrimNode &node)
{
	node.type = ToLuaType(c);
	node.data.dl = c;
}

inline void ToArrayPrimType(short s, ArrayPrimNode &node)
{
	node.type = ToLuaType(s);
	node.data.dl = s;
}

inline void ToArrayPrimType(unsigned short s, ArrayPrimNode &node)
{
	node.type = ToLuaType(s);
	node.data.dl = s;
}

inline void ToArrayPrimType(int n, ArrayPrimNode &node)
{
	node.type = ToLuaType(n);
	node.data.dl = n;
}

inline void ToArrayPrimType(unsigned int n, ArrayPrimNode &node)
{
	node.type = ToLuaType(n);
	node.data.dl = n;
}

inline void ToArrayPrimType(long l, ArrayPrimNode &node)
{
	node.type = ToLuaType(l);
	node.data.dl = l;
}

inline void ToArrayPrimType(unsigned long l, ArrayPrimNode &node)
{
	node.type = ToLuaType(l);
	node.data.dl = l;
}

inline void ToArrayPrimType(float f, ArrayPrimNode &node)
{
	node.type = ToLuaType(f);
	node.data.dl = f;
}

inline void ToArrayPrimType(double d, ArrayPrimNode &node)
{
	node.type = ToLuaType(d);
	node.data.dl = d;
}

inline void ToArrayPrimType(const char *str, ArrayPrimNode &node)
{
	node.type = ToLuaType(str);
	node.data.ptr = (void*)str;
}

inline void ToArrayPrimType(void *p, ArrayPrimNode &node)
{
	node.type = ToLuaType(p);
	node.data.ptr = p;
}

// 将ArrayPrimNode中数据转换成指定类型
template <typename T>
	bool ArrayPrimNodeConvert(const ArrayPrimNode &node, T &t)
{
	if(node.type == ToLuaType(t))
	{
		switch(node.type)
		{
		case LUA_TSTRING:
			t = (const char*)node.data.ptr;
			break;
		case LUA_TLIGHTUSERDATA:
			t = node.data.ptr;
			break;
		default:
			t = static_cast<T>(node.data.dl);
		}
		return true;
	}

	return false;
}


//! 在C++于LUA之间进行数组形式数据交换的类型
class LuaArray
{
public:
	//! 需要在lua全局环境中初始化
	static void initialize(lua_State *L);

	//! 填充一个ArrayPrimNode列表
	template <typename T>
		static void push2Vec(const T &v, std::vector<ArrayPrimNode> &vec)
	{
		ArrayPrimNode node;
		ToArrayPrimType(v, node);
		vec.push_back(node);
	}

	//! 由vec创建一个Array以userdata的形式push到L的stack上
	static void pushArray2Lua(lua_State *L, std::vector<ArrayPrimNode> &vec);

	//! 由userdata转换为ArrayPrimNode列表
	//注： 确保userdata处于栈顶
	static bool userdata2Array(lua_State *L, int idx, std::vector<ArrayPrimNode> &vec);

	//////////////////////////////////////////////////////////////////////////
	// internal!! you should never call these functions

	static int pack_size(lua_State *L);

	static int pack_new(lua_State *L);

	static int proxy_get(lua_State *L);

	static int proxy_set(lua_State *L);

private:
	static ArrayPrimNode* internal_check(lua_State *L, int idx);

	//! 此函数将创建一个userdata并push到L的stack
	static ArrayPrimNode* internal_create(lua_State *L, int nNodes);

};


#endif //end __LUA_ARRAY_H__