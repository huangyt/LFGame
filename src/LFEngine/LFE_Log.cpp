#include "LFE_Log.h"

#include "LFE_System.h"

namespace LF
{
	#define CONSOLE_TILE	L"LFEngine debug window"
	#define MAX_BUF_LEN		4096

	CLFE_Logger g_Logger;

	CLFE_Logger::CLFE_Logger()
	{
		m_hLogFile = NULL;

#ifdef _DEBUG
		m_hConsole = NULL;
		if(AllocConsole())
		{
			SetConsoleTitle(CONSOLE_TILE);
			//以下代码防止关闭控制台窗口   
			HWND hwnd = NULL;
			while(NULL == hwnd)
			{
				hwnd = ::FindWindow(NULL, CONSOLE_TILE);  
			} 
			HMENU hmenu = ::GetSystemMenu(hwnd, FALSE);   
			DeleteMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);  

			m_hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleMode(m_hConsole, ENABLE_PROCESSED_OUTPUT);
		}
#endif

		CreateDirectory(L"Logs", NULL);

		SYSTEMTIME tm;
		GetLocalTime(&tm);
		std::wstring strTime = std::move(Format(L"Logs//%d-%02d-%02d %02d_%02d_%02d_%03d.txt", 
			tm.wYear, tm.wMonth, tm.wDay,
			tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds));

		m_hLogFile = CreateFile(strTime.c_str(), GENERIC_WRITE, 0, 
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(m_hLogFile == INVALID_HANDLE_VALUE)
		{
			m_hLogFile = NULL;
		}
		else
		{
			WORD a=0xfeff;
			DWORD dwWritten = 0;
			WriteFile(m_hLogFile,&a,sizeof(WORD),&dwWritten,NULL);
		}
	}

	CLFE_Logger::~CLFE_Logger()
	{
#ifdef _DEBUG
		if(m_hConsole)
		{
			FreeConsole();
			CloseHandle(m_hConsole);
		}
#endif

		if(m_hLogFile)
		{
			CloseHandle(m_hLogFile);
		}
	}

	void CLFE_Logger::Print(const std::wstring& strOutPut)
	{
		DWORD dwWritten = 0;

#ifdef _DEBUG
		if(m_hConsole)
		{
			WriteConsole(m_hConsole,strOutPut.c_str(), strOutPut.size(), &dwWritten, NULL);
		}
#endif

		if(m_hLogFile)
		{
			WriteFile(m_hLogFile, strOutPut.c_str(), sizeof(WCHAR)*strOutPut.size(),&dwWritten,NULL);
		}
	}

	void CLFEngine::Log(LOG_TYPE logType, std::wstring strLog, ...)
	{
		std::wstring strHead;
		switch(logType)
		{
		case LOG_DEBUG:
			{
				strHead = L"Debug";
				break;
			}
		case LOG_ERROR:
			{
				strHead = L"Error";
				break;
			}
		}
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		std::wstring strTime = std::move(Format(L"[%s]<%d-%02d-%02d %02d:%02d:%02d:%03d>: ", 
			strHead.c_str(), tm.wYear, tm.wMonth, tm.wDay, 
			tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds));

		strLog += L"\r\n";
		WCHAR message[MAX_BUF_LEN] = {0};
		va_list cur_arg;
		va_start(cur_arg, strLog);
		_vsnwprintf_s(message, MAX_BUF_LEN, strLog.c_str(), cur_arg);
		va_end(cur_arg);

		if(logType == LOG_ERROR)
		{
			pSystem->m_strError = message;
		}

		g_Logger.Print(strTime + message);
	}
}