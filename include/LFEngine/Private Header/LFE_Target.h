#pragma once

#include "LFEngine.h"
#include <d3dx9.h>

namespace LF
{
	class CLFE_Texture;
	class CLFE_Target : public CLFTarget
	{
		friend class CLFE_Video;
	public:
		virtual CLFTexture*		GetTexture();

		virtual void			Release();

	private:
		CLFE_Target();
		~CLFE_Target();

	private:
		int					m_nWidth;
		int					m_nHeight;
		IDirect3DSurface9*	m_pDepth;
		CLFE_Texture*		m_pLFETexture;
	};
}