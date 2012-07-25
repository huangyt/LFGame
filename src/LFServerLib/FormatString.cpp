#include "FormatString.h"

std::string strFormat(const std::string strFmt, ...)
{
	char buff[4096] = {0};
	va_list cur_arg;
	va_start(cur_arg, strFmt);
	_vsnprintf_s(buff, 4096, strFmt.c_str(), cur_arg);
	va_end(cur_arg);
	return buff;
}

std::wstring wstrFormat(const std::wstring strFmt, ...)
{
	WCHAR buff[4096] = {0};
	va_list vl;
	va_start (vl, strFmt);
	_vsnwprintf_s (buff, 10240, strFmt.c_str(), vl);
	va_end (vl);
	return buff;
}

std::string FormatIP(DWORD dwIP)
{
	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = dwIP;
	return std::move(strFormat("%s", inet_ntoa(local.sin_addr)));
}

std::wstring MBToUnicode(const std::string &strMB)
{
	int nLen = MultiByteToWideChar(CP_ACP, 0, strMB.c_str(), strMB.size(), NULL, 0);
	if(nLen <= 0)
	{
		return NULL;
	}

	WCHAR *pStrUnicode = new WCHAR[nLen + 1];
	if(pStrUnicode == NULL)
	{
		return NULL;
	}

	MultiByteToWideChar(CP_ACP, 0, strMB.c_str(), strMB.size(), pStrUnicode, nLen);
	pStrUnicode[nLen] = 0;

	if(pStrUnicode[0] == 0xFEFF)
	{
		for(int i = 0; i < nLen; i++) 
		{
			pStrUnicode[i] = pStrUnicode[i+1]; 
		}
	}
	std::wstring strUnicode(pStrUnicode);
	delete [] pStrUnicode;
	return std::move(strUnicode);
}

std::string UnicodeToMB(const std::wstring &strUnicode)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, strUnicode.c_str(), -1, NULL, 0, NULL, NULL);
	if (nLen<= 0)
	{
		return NULL;
	}

	char *pStrMB = new char[nLen];
	if(pStrMB == NULL)
	{
		return NULL;
	}

	WideCharToMultiByte(CP_ACP, 0, strUnicode.c_str(), -1, pStrMB, nLen, NULL, NULL);
	pStrMB[nLen -1] = 0;
	std::string strMB(pStrMB);
	delete [] pStrMB;
	return std::move(strMB);
}