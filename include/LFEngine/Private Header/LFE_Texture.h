#pragma once

#include "LFEngine.h"
#include <d3dx9.h>

namespace LF
{
	class CLFE_Texture : public CLFTexture
	{
		friend class CLFE_Video;
		friend class CLFE_Target;
		friend class CLFE_ResManager;
	public:
		virtual int				GetWidth(BOOL bOriginal =FALSE);

		virtual int				GetHeight(BOOL bOriginal =FALSE);

		virtual void			Release();

		virtual DWORD*			Lock(BOOL bReadOnly = FALSE, 
			int nLeft	= 0, int nTop		= 0,
			int nWidth	= 0, int nHeight	= 0);

		virtual void			Unlock();
	private:
		CLFE_Texture();
		~CLFE_Texture();

	private:
		LPDIRECT3DTEXTURE9	m_pTexture;
		int					m_nWidth;
		int					m_nHeight;
	};
}