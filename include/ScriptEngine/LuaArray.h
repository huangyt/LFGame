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
// ��C++���ͻ�ȡ��Ӧ��Lua����

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
// ��C++��������д��ArrayPrimNode

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

// ��ArrayPrimNode������ת����ָ������
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


//! ��C++��LUA֮�����������ʽ���ݽ���������
class LuaArray
{
public:
	//! ��Ҫ��luaȫ�ֻ����г�ʼ��
	static void initialize(lua_State *L);

	//! ���һ��ArrayPrimNode�б�
	template <typename T>
		static void push2Vec(const T &v, std::vector<ArrayPrimNode> &vec)
	{
		ArrayPrimNode node;
		ToArrayPrimType(v, node);
		vec.push_back(node);
	}

	//! ��vec����һ��Array��userdata����ʽpush��L��stack��
	static void pushArray2Lua(lua_State *L, std::vector<ArrayPrimNode> &vec);

	//! ��userdataת��ΪArrayPrimNode�б�
	//ע�� ȷ��userdata����ջ��
	static bool userdata2Array(lua_State *L, int idx, std::vector<ArrayPrimNode> &vec);

	//////////////////////////////////////////////////////////////////////////
	// internal!! you should never call these functions

	static int pack_size(lua_State *L);

	static int pack_new(lua_State *L);

	static int proxy_get(lua_State *L);

	static int proxy_set(lua_State *L);

private:
	static ArrayPrimNode* internal_check(lua_State *L, int idx);

	//! �˺���������һ��userdata��push��L��stack
	static ArrayPrimNode* internal_create(lua_State *L, int nNodes);

};


#endif //end __LUA_ARRAY_H__