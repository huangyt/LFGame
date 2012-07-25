#pragma once

#include "GlobalDef.h"

std::string strFormat(const std::string strFmt, ...);

std::wstring wstrFormat(const std::wstring strFmt, ...);

std::wstring MBToUnicode(const std::string &strMB);

std::string UnicodeToMB(const std::wstring &strUnicode);

std::string FormatIP(DWORD dwIP);