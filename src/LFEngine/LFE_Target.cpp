#include "LFE_Target.h"

#include "LFE_System.h"
#include "LFE_Texture.h"
#include "LFE_Video.h"

namespace LF
{
	CLFE_Target::CLFE_Target()
	{
		m_pDepth = NULL;
		m_pLFETexture = NULL;
	}

	CLFE_Target::~CLFE_Target()
	{
		if(m_pLFETexture)
		{
			SAFE_DELETE(m_pLFETexture);
		}

		if(m_pDepth)
		{
			SAFE_RELEASE(m_pDepth);
		}
	}

	CLFTexture* CLFE_Target::GetTexture()
	{
		return reinterpret_cast<CLFTexture*>(m_pLFETexture);
	}

	void CLFE_Target::Release()
	{
		for(CLFE_TarVec::iterator iter = pSystem->m_pVideo->m_TargetList.begin();
			iter != pSystem->m_pVideo->m_TargetList.end(); ++iter)
		{
			if(this == *iter)
			{
				pSystem->m_pVideo->m_TargetList.erase(iter);
				delete this;
				break;
			}
		}
	}
}