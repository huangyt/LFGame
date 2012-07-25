#include "LFE_ResManager.h"

#include "LFE_System.h"
#include "LFE_Video.h"
#include "LFE_Texture.h"
#include "LFE_Font.h"

namespace LF
{
	CLFE_ResManager::CLFE_ResManager()
	{

	}

	CLFE_ResManager::~CLFE_ResManager()
	{

	}

	LPVOID CLFE_ResManager::ReadFromFile(const std::wstring &strFileName, DWORD &nFileSize)
	{
		HANDLE hF;
		hF = CreateFile(strFileName.c_str(), 
			GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
		if(hF == INVALID_HANDLE_VALUE)
		{
			pSystem->Log(LOG_ERROR, L"Can't load resource: %s", strFileName.c_str());
			return NULL;
		}
		DWORD nSize = GetFileSize(hF, NULL);

		LPVOID *ptr = new LPVOID[nSize];
		if(!ptr)
		{
			CloseHandle(hF);
			pSystem->Log(LOG_ERROR, L"Can't load resource: %s", strFileName.c_str());
			return NULL;
		}
		if(ReadFile(hF, ptr, nSize, &nSize, NULL ) == 0)
		{
			CloseHandle(hF);
			SAFE_DELETE_ARRAY(ptr);
			pSystem->Log(LOG_ERROR, L"Can't load resource: %s", strFileName.c_str());
			return NULL;
		}
		CloseHandle(hF);
		nFileSize = nSize;
		return ptr;
	}

	CLFTexture*	CLFE_ResManager::LoadTexture(const std::wstring &strFileName, BOOL bMipmap)
	{
		LPVOID data;
		DWORD size;
		data = ReadFromFile(strFileName, size);
		CLFTexture* pTex =  LoadTextureFromMemory(data, size, bMipmap);

		if(data)
		{
			SAFE_DELETE_ARRAY(data);
		}
		return pTex;
	}

	CLFTexture*	CLFE_ResManager::LoadTextureFromMemory(const LPVOID pBuff, DWORD nBuffSize, BOOL bMipmap)
	{
		if(!pBuff)
		{
			return NULL;
		}

		D3DFORMAT fmt1, fmt2;
		if(*(DWORD*)pBuff == 0x20534444) // Compressed DDS format magic number
		{
			fmt1=D3DFMT_UNKNOWN;
			fmt2=D3DFMT_A8R8G8B8;
		}
		else
		{
			fmt1=D3DFMT_A8R8G8B8;
			fmt2=D3DFMT_UNKNOWN;
		}

		CLFE_Texture *pTex = new CLFE_Texture;

		if(!pTex)
		{
			pSystem->Log(LOG_DEBUG, L"Can't create texture");
			return NULL;
		}

		D3DXIMAGE_INFO info;
		if( FAILED( D3DXCreateTextureFromFileInMemoryEx( pSystem->m_pVideo->GetDevice(), 
			pBuff, nBuffSize,
			D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
			bMipmap ? 0:1,		// Mip levels
			0,					// Usage
			fmt1,				// Format
			D3DPOOL_MANAGED,	// Memory pool
			D3DX_FILTER_NONE,	// Filter
			D3DX_DEFAULT,		// Mip filter
			0,					// Color key
			&info, NULL,
			&pTex->m_pTexture ) ) )
		{

			if( FAILED( D3DXCreateTextureFromFileInMemoryEx( pSystem->m_pVideo->GetDevice(), 
				pBuff, nBuffSize,
				D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
				bMipmap ? 0:1,		// Mip levels
				0,					// Usage
				fmt2,				// Format
				D3DPOOL_MANAGED,	// Memory pool
				D3DX_FILTER_NONE,	// Filter
				D3DX_DEFAULT,		// Mip filter
				0,					// Color key
				&info, NULL,
				&pTex->m_pTexture ) ) )
			{
				pSystem->Log(LOG_DEBUG, L"Can't create texture");
				return NULL;
			}
		}
		pTex->m_nWidth = info.Width;
		pTex->m_nHeight = info.Height;

		pSystem->m_pVideo->m_TextureList.push_back(pTex);

		return reinterpret_cast<CLFTexture*>(pTex);
	}

	CLFFont* CLFE_ResManager::LoadFont(const std::wstring &strFontName,const FontDes &ftDes)
	{
		CLFE_Font *pFont = pSystem->m_pVideo->m_pFontManager->CreateFont(strFontName, ftDes);
		return reinterpret_cast<CLFFont*>(pFont);
	}
}