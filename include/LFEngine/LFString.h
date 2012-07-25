#pragma once

#include <string>
#include <sstream>

namespace LF
{
	std::wstring Format(const std::wstring strFormat, ...);

	std::wstring MBToUnicode(const std::string &strMB);

	std::string UnicodeToMB(const std::wstring &strUnicode);
}