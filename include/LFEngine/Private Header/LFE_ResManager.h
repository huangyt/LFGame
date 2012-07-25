#pragma once

#include "LFEngine.h"

namespace LF
{
	//资源管理器
	class CLFE_ResManager : public CLFResManager
	{
		friend class CLFE_System;
	public:
		virtual CLFTexture*		LoadTexture(const std::wstring &strFileName, BOOL bMipmap);

		virtual CLFTexture*		LoadTextureFromMemory(const LPVOID pBuff, DWORD nBuffSize, BOOL bMipmap);

		virtual CLFFont*		LoadFont(const std::wstring &strFontName,const FontDes &ftDes);
	private:
		LPVOID					ReadFromFile(const std::wstring &strFileName, DWORD &nFileSize);

	private:
		CLFE_ResManager();
		~CLFE_ResManager();
	};
}