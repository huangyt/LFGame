#include <iostream>
#include "ScriptEngine.h"
#include "Luaclass.h"
#include "LuaFunction.h"
#include "LuaPackage.h"
#include <Windows.h>
#include "TestBase.h"

int main()
{
	pt("****************");
	pt("C++与lua交互测试");
	pt("按1、2测试脚本");
	pt("按Q退出测试");
	pt("****************");
	while(true)
	{
		if(GetAsyncKeyState('Q'))
			break;

		if(GetAsyncKeyState('1'))
		{
			Test1 test;
			test.Init();
			test.Start();
			test.End();
		}

		if(GetAsyncKeyState('2'))
		{
			Test2 test;
			test.Init();
			test.Start();
			test.End();
		}

		if(GetAsyncKeyState('3'))
		{
			Test3 test;
			test.Init();
			test.Start();
			test.End();
		}

		system("pause");
	}
}