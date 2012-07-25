#pragma once

#include "LFEngine.h"

namespace LF
{
	class CLFE_Logger
	{
		friend class CLFE_System;
	public:
		CLFE_Logger();
		~CLFE_Logger();

		void Print(const std::wstring& strOutPut);
	private:
#ifdef _DEBUG
		HANDLE m_hConsole;
#endif
		HANDLE m_hLogFile;	
	};
}