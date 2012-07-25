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
	pt("C++��lua��������");
	pt("��1��2���Խű�");
	pt("��Q�˳�����");
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