#pragma once

#include <Windows.h>

namespace LFG
{
	class CLFGui
	{
	public:
		static CLFGui* CreateGui();

	public:
		virtual void Release() = 0;

		virtual BOOL MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	};
}