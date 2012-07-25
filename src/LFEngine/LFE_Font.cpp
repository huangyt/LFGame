#include "LFE_Font.h"

#include "LFE_System.h"
#include "LFE_Texture.h"
#include "LFE_Video.h"

namespace LF
{
	void RasterCallback(const int y,const int count,const FT_Span * const spans,void * const user) 
	{
		Spans *sptr = (Spans *)user;
		for (int i = 0; i < count; ++i) 
			sptr->push_back(Span(spans[i].x, y, spans[i].len, spans[i].coverage));
	}

	void RenderSpans(FT_Library &library,FT_Outline * const outline, Spans *spans) 
	{
		FT_Raster_Params params;
		memset(&params, 0, sizeof(params));
		params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
		params.gray_spans = RasterCallback;
		params.user = spans;

		FT_Outline_Render(library, outline, &params);
	}

	CLFE_Font::CLFE_Font(UINT nCharWidth, 
						UINT nCharHeight,
						BOOL bAntiAliased,
						UINT nStrokeSize,
						DWORD dwStrokeColor,
						DWORD dwStrokeFontColor)
		:m_nCharWidth(nCharWidth+nStrokeSize*2)
		,m_nCharHeight(nCharHeight+nStrokeSize*2)
		,m_bAntiAliased(bAntiAliased)
		,m_nStrokeSize(nStrokeSize)
		,m_dwStrokeColor(dwStrokeColor)
		,m_dwStrokeFontColor(dwStrokeFontColor)
		,m_pFTFace(NULL)
	{
		m_nFontTexWidth = (m_nCharWidth+CHAR_SPACE)*FONT_SIZE_W;
		m_nFontTexHeight = (m_nCharHeight+CHAR_SPACE)*FONT_SIZE_H;

		CLFTexture *pTemp = pSystem->m_pVideo->CreateTexture(m_nFontTexWidth, m_nFontTexHeight);
		m_pFontTex = reinterpret_cast<CLFE_Texture*>(pTemp);
	}

	CLFE_Font::~CLFE_Font()
	{
		if(m_pFontTex)
		{
			m_pFontTex->Release();
		}
		FT_Done_Face(m_pFTFace);
	}

	void CLFE_Font::Draw(const std::wstring &strText, 
						pos_i &pt, 
						DWORD Color,
						float z)
	{
		pos_i pos = pt;
		for(std::size_t i=0; i < strText.length();++i)
		{
			FT_Long lCharCode = strText[i];
			//¿Õ¸ñ·û£¬¿í¶ÈÎª°ë¸ö×Ö·û¿í¶È
			if(lCharCode==0x20)
			{
				pos.X += m_nCharWidth/2;
				continue;
			}
			//»»ÐÐ·û
			if(lCharCode==0xa)
			{
				pos.X = pt.X;
				pos.Y += m_nCharHeight;
				continue;
			}

			const FONTNODE* node = this->Check(lCharCode);
			if(node)
			{
				Quad quad;
				if(m_nStrokeSize==0)
				{
					quad.setQuadColor(Color);
				}

				quad.setQuadRect(&rect_f(pos.X+node->_nOffSetX,pos.Y+node->_nOffSetY, 
					pos.X+node->_nOffSetX+m_nCharWidth, pos.Y+node->_nOffSetY+m_nCharHeight));
				quad.setZBuffer(z);
				quad.pTex = m_pFontTex;
				quad.blend = BLEND_DEFAULT_Z;

				int nX = node->_nX*(m_nCharWidth+CHAR_SPACE);
				int nY = node->_nY*(m_nCharHeight+CHAR_SPACE);
				quad.setTextureRect(&rect_f(nX,nY,nX+m_nCharWidth, nY+m_nCharHeight));

				pSystem->m_pVideo->RenderQuad(&quad);

				pos.X += node->_nAdvance;
			}
		}
	}

	const FONTNODE* CLFE_Font::Check(FT_Long lCharCode)
	{
		int nHashKey = lCharCode % FONTARRAY_LENGTH;
		int nFontIndex = -1;
		for(int i=0;;++i)
		{
			int nLeft = nHashKey-i*i;
			int nRight = nHashKey+i*i;

			if(nLeft<0 && nRight>=FONTARRAY_LENGTH)
			{
				pSystem->m_pVideo->RenderQuad(&Quad());
				this->ClearCharTexture(nHashKey);
				if(this->AddCharTexture(lCharCode, nHashKey))
				{
					nFontIndex = nHashKey;
				}
				pSystem->Log(LOG_DEBUG, L"Òç³ö£º%i", lCharCode);
				break;
			}

			if(nLeft>=0)
			{
				if(m_FontNodeArray[nLeft]._lCharCode==-1)
				{
					if(this->AddCharTexture(lCharCode, nLeft))
					{
						nFontIndex = nLeft;
					}
					break;
				}
				else
				if(m_FontNodeArray[nLeft]._lCharCode==lCharCode)
				{
					nFontIndex = nLeft;
					break;
				}
			}

			if(nRight<FONTARRAY_LENGTH)
			{
				if(m_FontNodeArray[nRight]._lCharCode==-1)
				{
					if(this->AddCharTexture(lCharCode, nRight))
					{
						nFontIndex = nRight;
					}
					break;
				}
				else
				if(m_FontNodeArray[nRight]._lCharCode==lCharCode)
				{
					nFontIndex = nRight;
					break;
				}
			}
		}

		if(nFontIndex==-1)
		{
			return NULL;
		}
		return &m_FontNodeArray[nFontIndex];
	}

	void CLFE_Font::ClearCharTexture(UINT nIndex)
	{
		UINT nCurTexPosY = nIndex/FONT_SIZE_H;
		UINT nCurTexPosX = nIndex%FONT_SIZE_W;

		DWORD *pTx = m_pFontTex->Lock(FALSE, nCurTexPosX*(m_nCharWidth+CHAR_SPACE), 
			nCurTexPosY*(m_nCharHeight+CHAR_SPACE),
			(m_nCharWidth+CHAR_SPACE), (m_nCharHeight+CHAR_SPACE));

		for(UINT i = 0; i<(m_nCharHeight+CHAR_SPACE);++i)
		{
			for (UINT j = 0; j<(m_nCharWidth+CHAR_SPACE);++j)
			{
				pTx[i*m_nFontTexWidth+j] = 0x00ffffff;
			}
		}

		m_pFontTex->Unlock();
	}

	BOOL CLFE_Font::AddCharTexture(FT_Long lCharCode, UINT nIndex)
	{
		if(m_nStrokeSize>0)
		{
			return this->AddCharTextureWithStroke(lCharCode, nIndex);
		}

		if(m_pFTFace && FT_Load_Char(m_pFTFace,   lCharCode,   FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT
			|(m_bAntiAliased ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO)))
		{
			return FALSE;
		}

		UINT nCurTexPosY = nIndex/FONT_SIZE_H;
		UINT nCurTexPosX = nIndex%FONT_SIZE_W;

		FT_GlyphSlot slot = m_pFTFace->glyph;
		FT_Bitmap& bitmap = slot->bitmap;

		DWORD *pTx = m_pFontTex->Lock(FALSE, nCurTexPosX*(m_nCharWidth+CHAR_SPACE), 
			nCurTexPosY*(m_nCharHeight+CHAR_SPACE),
			bitmap.width, bitmap.rows);
		switch(bitmap.pixel_mode)
		{
		case FT_PIXEL_MODE_GRAY:
			{
				for(int i = 0; i<bitmap.rows;++i)
				{
					for (int j = 0; j<bitmap.width;++j)
					{
						pTx[i*m_nFontTexWidth+j] = bitmap.buffer[i*bitmap.pitch+j]<<24 | 0x00ffffff;
					}
				}
				break;
			}
		case FT_PIXEL_MODE_MONO:
			{
				for(int i = 0; i<bitmap.rows;++i)
				{
					UCHAR *src = bitmap.buffer + (i * bitmap.pitch);
					for (int j = 0; j<bitmap.width;++j)
					{
						pTx[i*m_nFontTexWidth+j] = 
							(src [j / 8] & (0x80 >> (j & 7))) ? 0xffffffff : 0x00ffffff;
					}
				}
				break;
			}
		}
		m_pFontTex->Unlock();

		//×Ö·ûÌí¼Ó
		m_FontNodeArray[nIndex]._lCharCode = lCharCode;
		m_FontNodeArray[nIndex]._nX = nCurTexPosX;
		m_FontNodeArray[nIndex]._nY = nCurTexPosY;
		m_FontNodeArray[nIndex]._nAdvance = slot->metrics.horiAdvance>>6;
		m_FontNodeArray[nIndex]._nOffSetX = slot->metrics.horiBearingX>>6;
		m_FontNodeArray[nIndex]._nOffSetY = 
			(m_pFTFace->size->metrics.descender>>6)+m_nCharHeight-(slot->metrics.horiBearingY>>6);
		return TRUE;
	}

	BOOL CLFE_Font::AddCharTextureWithStroke(FT_Long lCharCode, UINT nIndex)
	{
		FT_Error ftError;
		ftError = FT_Load_Char(m_pFTFace, lCharCode, FT_LOAD_NO_BITMAP | FT_LOAD_FORCE_AUTOHINT | 
			(m_bAntiAliased ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_MONO));
		if(ftError || m_pFTFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		{
			return FALSE;
		}

		UINT nCurTexPosY = nIndex/FONT_SIZE_H;
		UINT nCurTexPosX = nIndex%FONT_SIZE_W;

		FT_GlyphSlot slot = m_pFTFace->glyph;
		FT_Library &ftLib = pSystem->m_pVideo->m_pFontManager->m_pLibrary;
		Spans spans;
		RenderSpans(ftLib, &m_pFTFace->glyph->outline, &spans);
		Spans outlineSpans;

		FT_Stroker stroker;
		FT_Stroker_New(ftLib, &stroker);
		FT_Stroker_Set(stroker,
			(int)(m_nStrokeSize * 64),
			FT_STROKER_LINECAP_ROUND,
			FT_STROKER_LINEJOIN_ROUND,
			0);

		FT_Glyph glyph;
		ftError = FT_Get_Glyph(m_pFTFace->glyph, &glyph);
		if(ftError)
		{
			return FALSE;
		}

		FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

		if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
		{
			FT_Outline *o = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
			RenderSpans(ftLib, o, &outlineSpans);
		}

		FT_Stroker_Done(stroker);
		FT_Done_Glyph(glyph);

		if (!spans.empty())
		{
			FontRect rect(spans.front().x,
				spans.front().y,
				spans.front().x,
				spans.front().y);
			for (Spans::iterator s = spans.begin();
				s != spans.end(); ++s)
			{
				rect.Include(s->x, s->y);
				rect.Include(s->x + s->width - 1, s->y);
			}
			for (Spans::iterator s = outlineSpans.begin();
				s != outlineSpans.end(); ++s)
			{
				rect.Include(s->x, s->y);
				rect.Include(s->x + s->width - 1, s->y);
			}

			int imgWidth = rect.Width(),
				imgHeight = rect.Height(),
				imgSize = imgWidth * imgHeight;

			DWORD *pTx = m_pFontTex->Lock(FALSE, nCurTexPosX*(m_nCharWidth+CHAR_SPACE), 
				nCurTexPosY*(m_nCharHeight+CHAR_SPACE), imgWidth, imgHeight);

			for (Spans::iterator s = outlineSpans.begin();s != outlineSpans.end(); ++s)
			{
				for (int w = 0; w < s->width; ++w)
				{
					pTx[(int)((imgHeight - 1 - (s->y - rect.ymin)) * m_nFontTexWidth
						+ s->x - rect.xmin + w)] = 
						ARGB(s->coverage, 
						GETR(m_dwStrokeColor), 
						GETG(m_dwStrokeColor),
						GETB(m_dwStrokeColor));
				}
			}

			for (Spans::iterator s = spans.begin();s != spans.end(); ++s)
			{
				for (int w = 0; w < s->width; ++w)
				{
					DWORD dstCol = pTx[(int)((imgHeight - 1 - (s->y - rect.ymin)) * 
						m_nFontTexWidth + s->x - rect.xmin + w)];

					BYTE dstA = GETA(dstCol);
					BYTE dstR = GETR(dstCol);
					BYTE dstG = GETG(dstCol);
					BYTE dstB = GETB(dstCol);

					BYTE srcA = s->coverage;
					BYTE srcR = GETR(m_dwStrokeFontColor);
					BYTE srcG = GETG(m_dwStrokeFontColor);
					BYTE srcB = GETB(m_dwStrokeFontColor);

					dstR = (int)(dstR + ((srcR - dstR) * srcA) / 255.0f);
					dstG = (int)(dstG + ((srcG - dstG) * srcA) / 255.0f);
					dstB = (int)(dstB + ((srcB - dstB) * srcA) / 255.0f);
					dstA = __min(255, dstA + srcA);

					pTx[(int)((imgHeight - 1 - (s->y - rect.ymin)) * m_nFontTexWidth
						+ s->x - rect.xmin + w)] = ARGB(dstA,dstR,dstG,dstB);
				}
			}

			m_pFontTex->Unlock();

			//×Ö·ûÌí¼Ó
			m_FontNodeArray[nIndex]._lCharCode = lCharCode;
			m_FontNodeArray[nIndex]._nX = nCurTexPosX;
			m_FontNodeArray[nIndex]._nY = nCurTexPosY;
			m_FontNodeArray[nIndex]._nAdvance = (slot->metrics.horiAdvance>>6)+m_nStrokeSize;
			m_FontNodeArray[nIndex]._nOffSetX = slot->metrics.horiBearingX>>6;
			m_FontNodeArray[nIndex]._nOffSetY = 
				(m_pFTFace->size->metrics.descender>>6)+m_nCharHeight-(slot->metrics.horiBearingY>>6)-m_nStrokeSize*2;
			return TRUE;
		}
		return FALSE;
	}

	void CLFE_Font::Release()
	{
		CLFE_FontVec &fontList = pSystem->m_pVideo->m_pFontManager->m_FontList;
		for(CLFE_FontVec::iterator iter = fontList.begin();
			iter != fontList.end(); ++iter)
		{
			if(*iter == this)
			{
				fontList.erase(iter);
				delete this;
				break;
			}
		}
	}

	CLFE_FontManager::CLFE_FontManager()
	{
		m_pLibrary = NULL;
		FT_Init_FreeType(&m_pLibrary);
	}

	CLFE_FontManager::~CLFE_FontManager()
	{
		for(CLFE_FontVec::iterator iter = m_FontList.begin();
			iter != m_FontList.end(); ++iter)
		{
			if(*iter)
			{
				SAFE_DELETE(*iter);
			}
		}
		FT_Done_FreeType(m_pLibrary);
	}
	
	CLFE_Font* CLFE_FontManager::CreateFont(const std::wstring &strFontName,const FontDes &ftDes)
	{
		UINT nFtWidth = ftDes._nWidth;
		UINT nFtHeight = ftDes._nHeight;
		if(nFtWidth>50)
		{
			nFtWidth = 50;
		}
		if(nFtHeight>50)
		{
			nFtHeight = 50;
		}
		CLFE_Font* pFont  = new CLFE_Font(nFtWidth, nFtHeight, 
			ftDes._bAntiAliased, ftDes._nStrokeSize, ftDes._dwStrokeColor, ftDes._dwStrokeFontColor);
		if(!m_pLibrary || !pFont)
		{
			return NULL;
		}

		if(FT_New_Face(m_pLibrary, UnicodeToMB(strFontName).c_str(), 0, &pFont->m_pFTFace))
		{
			SAFE_DELETE(pFont);
			return NULL;
		}
		if(!pFont->m_pFTFace->charmap || 
			FT_Set_Pixel_Sizes(pFont->m_pFTFace, nFtWidth, nFtHeight))
		{
			SAFE_DELETE(pFont);
			return NULL;
		}
		m_FontList.push_back(pFont);
		return pFont;
	}
}