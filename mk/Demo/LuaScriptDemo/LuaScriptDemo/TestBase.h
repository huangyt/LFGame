#pragma once

#include "Lua2UI.h"
#include "LuaFunction.h"
#include "LuaArray.h"
#include "LuaTable.h"

LUA_CALSS_TEMPLATE_IMPLEMENT(Dialog)

static EventPip s_EventPip;

class TestBase
{
public:
	TestBase(){}
	~TestBase(){}

	virtual void Init() = 0;

	virtual void Start() = 0;

	virtual void End() = 0;
};

class MyLuaScriptListener : public LuaScriptListener
{
public:
	void notify(LuaScript *p)
	{
		pt("notify:");
		char a[128] = {0};
		p->dumpError(a,128);
		pt(a);
	}
};

class Test1 : public TestBase
{
private:
	LuaScript* m_pLS;
	ScriptEngine* m_pSe;
public:
	Test1(){}
	~Test1(){}
	
	void Init()
	{
		pt("测试1：测试C++中函数和类导入到lua中！");
		pt("help:按Q退出这个测试！");

		m_pSe = new ScriptEngine();
		m_pLS = m_pSe->createScript();
		LuaClass<Dialog>::register_begin(m_pLS->getLuaState());
		LuaClass<Dialog>::register_mfunc("addChoice", &Dialog::addChoice);
		LuaClass<Dialog>::register_mfunc("startDialog", &Dialog::startDialog);
		LuaClass<Dialog>::register_mfunc<void (Dialog::*)(int)>("setID", &Dialog::setID);
		LuaClass<Dialog>::register_mfunc<void (Dialog::*)()>("setID1", &Dialog::setID);
		LuaClass<Dialog>::register_mfunc("setIDPtr", &Dialog::setIDPtr);
		LuaClass<Dialog>::register_mfunc("test", &Dialog::test);
		LuaClass<Dialog>::register_mfunc("getID", &Dialog::getID);
		LuaClass<Dialog>::register_end();

		LuaPackRegister rg;
		rg.beginPack(m_pLS->getLuaState(), "EventPip");
		rg.registerFunc("pop", EventPip::pop);
		rg.registerFunc("push", EventPip::push);
		rg.registerFunc("GetAsyncKeyState",EventPip::GetKS);
		rg.endPack();
	}

	void Start()
	{
		MyLuaScriptListener myLuaListener;
		if(m_pLS->doFile("test1.lua", &myLuaListener) > 1)
		{
			char a[128] = {0};
			m_pLS->dumpError(a,128);
			pt(a);
			return;
		}

		LuaFunction<void> luaFun_Update(m_pLS->getLuaState(),"Update");

		while(true)
		{
			if(m_pLS->getState() == ELSS_EMPTY || m_pLS->getState() == ELSS_ERROR)
			{
				break;
			}

			luaFun_Update(100);
			m_pSe->update((float)0.0001);
			Sleep(1000);
		}
	}

	void End()
	{
		m_pSe->discardScript(m_pLS);
		delete m_pSe;
		pt("测试1结束！");
	}
};

class Test2 : public TestBase
{
private:
	LuaScript* m_pLS;
	ScriptEngine* m_pSe;
public:
	Test2(){}
	~Test2(){}

	void Init()
	{
		pt("测试2：测试lua中函数和包中函数导入到C++中！");

		m_pSe = new ScriptEngine();
		m_pLS = m_pSe->createScript();
	}

	void Start()
	{
		if(m_pLS->doFile("test2.lua") != 0)
		{
			char a[128] = {0};
			m_pLS->dumpError(a,128);
			pt(a);
			return;
		}

		LuaFunction<float> luaFun_Add(m_pSe->getLuaState(),"add");
		pt(luaFun_Add("lifeng", 5));

		LuaFunction<void> luaFun_TableTest(m_pSe->getLuaState(),"test", "luaFun");
		luaFun_TableTest();

		LuaFunction<void> luaFun_Test(m_pSe->getLuaState(),"test");
		luaFun_Test("test - luaFun_Test");


		LuaFunction<void> luaFun_TableTest1(m_pSe->getLuaState(),"test1", "luaFun");
		luaFun_TableTest1();
		pt(luaFun_TableTest1.getLastErr());
	}

	void End()
	{
		m_pSe->discardScript(m_pLS);
		delete m_pSe;
		pt("测试2结束！");
	}
};
class Test3 : public TestBase
{
	typedef std::vector<ArrayPrimNode> LuaArrayVec;
private:
	LuaScript* m_pLS;
	ScriptEngine* m_pSe;
public:
	Test3(){}
	~Test3(){}

	static int PrintArray(lua_State *L)
	{
		LuaArrayVec testVec;
		LuaArray::userdata2Array(L, 1, testVec);
		
		for(LuaArrayVec::iterator iter = testVec.begin(); iter != testVec.end(); ++iter)
		{
			switch((*iter).type)
			{
			case LUA_TNIL:
				{
					pt("nil");
					break;
				}
			case LUA_TNUMBER:
				{
					pt((*iter).data.dl);
					break;
				}
			case LUA_TBOOLEAN:
				{
					pt((*iter).data.dl);
					break;
				}
			case LUA_TSTRING:
				{
					pt((char*)(*iter).data.ptr);
					break;
				}
			}
		}
		return 0;
	}

	static int GetArray(lua_State *L)
	{
		LuaArrayVec testArray;
		LuaArray::push2Vec("lifeng", testArray);
		LuaArray::push2Vec(123, testArray);
		ArrayPrimNode node;
		node.type = LUA_TNIL;
		node.data.dl = 0;
		testArray.push_back(node);
		LuaArray::pushArray2Lua(L,testArray);
		return 1;
	}

	void Init()
	{
		pt("测试3：测试LuaArray功能！");

		m_pSe = new ScriptEngine();
		m_pLS = m_pSe->createScript();

		lua_register(m_pSe->getLuaState(), "printArray", PrintArray);
		lua_register(m_pSe->getLuaState(), "getArray", GetArray);
	}

	void Start()
	{
		if(m_pLS->doFile("test3.lua") != 0)
		{
			char a[128] = {0};
			m_pLS->dumpError(a,128);
			pt(a);
			return;
		}

		if(!LuaTable::isGlobalFieldNil(m_pLS->getLuaState(), "tableTest"))
		{
			LuaTable::setGlobalField(m_pLS->getLuaState(), "tableTest", "test");
		}
		const char* pStrTable;
		if(LuaTable::getGlobalField(m_pSe->getLuaState(), "tableTest", pStrTable))
		{
			pt(pStrTable);
		}

		LuaTable lua_table("tableTest2", m_pSe->getLuaState());

		if(!lua_table.isNil("test"))
		{
			lua_table.setField("test", "wokao");
		}

		if(lua_table.getField("test", pStrTable))
		{
			pt(pStrTable);
		}

		LuaArrayVec test;
		char *pTest = (char*)lua_table.getRaw(3).data.ptr;

		lua_table.makeTableSnapshot(test);
		for(LuaArrayVec::iterator iter = test.begin(); iter != test.end(); ++iter )
		{
			pt((char*)iter->data.ptr);
		}
	}

	void End()
	{
		m_pSe->discardScript(m_pLS);
		delete m_pSe;
		pt("测试3结束！");
	}
};