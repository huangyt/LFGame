#pragma once

#include <Windows.h>
#include <string>
#include <unordered_map>

#include "LFGuiElement.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p){delete p;p=NULL;}
#endif

namespace LFG
{
	class CLFGui
	{
	public:
		CLFGui();
		~CLFGui();

		BOOL MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
	};
}