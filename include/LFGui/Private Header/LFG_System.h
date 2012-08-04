#pragma once

#include "LFGui.h"

namespace LFG
{
	class CLFG_System : public CLFGui
	{
		friend CLFGui* CLFGui::CreateGui();
	public:
		virtual void Release();

		virtual BOOL MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		CLFG_System();
		~CLFG_System();
	};
}