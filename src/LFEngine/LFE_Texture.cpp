#include "LFE_Texture.h"

#include "LFE_System.h"
#include "LFE_Video.h"

namespace LF
{
	CLFE_Texture::CLFE_Texture()
	{
		m_pTexture = NULL;
		m_nWidth = 0;
		m_nHeight = 0;
	}

	CLFE_Texture::~CLFE_Texture()
	{		
		if(m_pTexture)
		{
			SAFE_RELEASE(m_pTexture);
		}
	}

	int CLFE_Texture::GetWidth(BOOL bOriginal)
	{
		if(bOriginal)
		{
			return m_nWidth;
		}
		else
		{
			D3DSURFACE_DESC TDesc;
			if(!m_pTexture || FAILED(m_pTexture->GetLevelDesc(0, &TDesc))) 
			{
				return 0;
			}
			else 
			{
				return TDesc.Width;
			}
		}
	}

	int CLFE_Texture::GetHeight(BOOL bOriginal)
	{
		if(bOriginal)
		{
			return m_nHeight;
		}
		else
		{
			D3DSURFACE_DESC TDesc;
			if(!m_pTexture || FAILED(m_pTexture->GetLevelDesc(0, &TDesc))) 
			{
				return 0;
			}
			else 
			{
				return TDesc.Height;
			}
		}
	}

	DWORD* CLFE_Texture::Lock(BOOL bReadOnly /* = FALSE */, 
								int nLeft /* = 0 */, int nTop /* = 0 */, 
								int nWidth /* = 0 */, int nHeight /* = 0 */)
	{
		D3DSURFACE_DESC TDesc;
		D3DLOCKED_RECT TRect;
		RECT region, *prec;
		int flags;

		if(!m_pTexture)
		{
			return NULL;
		}

		m_pTexture->GetLevelDesc(0, &TDesc);
		if(TDesc.Format!=D3DFMT_A8R8G8B8 && TDesc.Format!=D3DFMT_X8R8G8B8) 
		{
			return NULL;
		}

		if(nWidth == 0)
		{
			nWidth = GetWidth();
		}

		if(nHeight == 0)
		{
			nHeight = GetHeight();
		}

		region.left=nLeft;
		region.top=nTop;
		region.right=nLeft+nWidth;
		region.bottom=nTop+nHeight;
		prec=&region;

		if(bReadOnly) 
		{
			flags=D3DLOCK_READONLY;
		}
		else 
		{
			flags=0;
		}

		if(FAILED(m_pTexture->LockRect(0, &TRect, prec, flags)))
		{
			pSystem->Log(LOG_ERROR, L"Can't lock texture");
			return NULL;
		}

		return (DWORD *)TRect.pBits;
	}

	void CLFE_Texture::Unlock()
	{
		if(m_pTexture)
		{
			m_pTexture->UnlockRect(0);
		}
	}

	void CLFE_Texture::Release()
	{
		for(CLFE_TexVec::iterator iter = pSystem->m_pVideo->m_TextureList.begin();
			iter != pSystem->m_pVideo->m_TextureList.end(); ++iter)
		{
			if(this == *iter)
			{
				pSystem->m_pVideo->m_TextureList.erase(iter);
				delete this;
				break;
			}
		}
	}
}