#include "LFG_System.h"

namespace LFG
{
	CLFGui* CLFGui::CreateGui()
	{
		return new CLFG_System;
	}

	CLFG_System::CLFG_System()
	{

	}

	CLFG_System::~CLFG_System()
	{

	}

	void CLFG_System::Release()
	{
		delete this;
	}

	BOOL CLFG_System::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return FALSE;
	}
}