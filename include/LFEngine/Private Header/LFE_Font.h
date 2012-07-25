#pragma once

// freetype2: 2.4.10

#include "LFEngine.h"
#include <ft2build.h> 
#include FT_FREETYPE_H
#include FT_STROKER_H

#define	FONTARRAY_LENGTH					887
#define FONT_SIZE_W							30
#define FONT_SIZE_H							30

#define CHAR_SPACE							1

namespace LF
{
	typedef struct	_fontNode
	{
		_fontNode()
		{
			_lCharCode = -1;
			_nX = 0;
			_nY = 0;
			_nAdvance = 0;
			_nOffSetX = 0;
			_nOffSetY = 0;
		}
		FT_Long	_lCharCode;				//×Ö·ûµÄunicodeÂë
		int		_nX;					//×Ö·ûºáÏò¸ñ×ÓÎ»ÖÃ
		int		_nY;					//×Ö·û×ÝÏò¸ñ×ÓÎ»ÖÃ
		int		_nAdvance;				//×Ö·û²½½ø
		int		_nOffSetX;				//×Ö·ûºáÏòÆ«ÒÆ
		int		_nOffSetY;				//×Ö·û×ÝÏòÆ«ÒÆ
	}FONTNODE;

	struct Span
	{
		Span() { }
		Span(int _x, int _y, int _width, int _coverage)
			: x(_x), y(_y), width(_width), coverage(_coverage) { }

		int x, y, width, coverage;
	};

	struct FontRect
	{
		FontRect() { }
		FontRect(float left, float top, float right, float bottom)
			: xmin(left), xmax(right), ymin(top), ymax(bottom) { }

		void Include(float x, float y)
		{
			xmin = __min(xmin, x);
			ymin = __min(ymin, y);
			xmax = __max(xmax, x);
			ymax = __max(ymax, y);
		}

		float Width() const { return xmax - xmin + 1; }
		float Height() const { return ymax - ymin + 1; }

		float xmin, xmax, ymin, ymax;
	};

	class CLFE_Texture;
	class CLFE_Font;
	class CLFE_FontManager;
	typedef std::vector<CLFE_Font*>		CLFE_FontVec;
	typedef std::vector<FONTNODE>		CLFE_OverFlowVec;
	typedef std::vector<Span>			Spans;

	class CLFE_Font : public CLFFont
	{
		friend class CLFE_FontManager;
	public:
		CLFE_Font(UINT nCharWidth, 
				UINT nCharHeight,
				BOOL bAntiAliased,
				UINT nStrokeSize,
				DWORD dwStrokeColor,
				DWORD dwStrokeFontColor);
		~CLFE_Font();

	public:
		virtual void			Draw(const std::wstring &strText, 
									pos_i &pt, 
									DWORD Color = 0xff000000,
									float z = 0.5f);

		virtual void			Release();

	private:
		const FONTNODE*			Check(FT_Long lCharCode);
		BOOL					AddCharTexture(FT_Long lCharCode, UINT nIndex);
		BOOL					AddCharTextureWithStroke(FT_Long lCharCode, UINT nIndex);
		void					ClearCharTexture(UINT nIndex);
	private:
		FT_Face						m_pFTFace;
		UINT						m_nCharWidth;
		UINT						m_nCharHeight;
		UINT						m_nFontTexWidth;
		UINT						m_nFontTexHeight;
		UINT						m_nStrokeSize;
		DWORD						m_dwStrokeColor;
		DWORD						m_dwStrokeFontColor;
		BOOL						m_bAntiAliased;

		CLFE_Texture*				m_pFontTex;
		FONTNODE					m_FontNodeArray[FONTARRAY_LENGTH];
	};

	class CLFE_FontManager
	{
	public:
		CLFE_FontManager();
		~CLFE_FontManager();

		CLFE_Font*				CreateFont(const std::wstring &strFontName,const FontDes &ftDes);
	public:
		CLFE_FontVec			m_FontList;
		FT_Library				m_pLibrary;
	};
}