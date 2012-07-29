#pragma once

#include <Windows.h>
#include <vector>
#include <math.h>

#include "LFDefine.h"
#include "LFString.h"
#include "LFMath.h"

#pragma warning (disable:4244)

namespace LF
{
	class CLFVideo;
	class CLFInput;
	class CLFResManager;
	//引擎接口
	class CLFEngine
	{
	public:
		static 	CLFEngine*			CreateEngine(); 

		static	void				Log(LOG_TYPE logType, std::wstring strLog, ...);

	public:
		virtual BOOL				Initiate()			= 0;

		virtual BOOL				Run()				= 0;

		virtual void				ShutDown()			= 0;

		virtual void				Release()			= 0;

		virtual std::wstring		GetErrorMessage()	= 0;

		virtual CLFVideo*			GetVideoPtr()		= 0;

		virtual CLFResManager*		GetResManagerPtr()	= 0;

		virtual CLFInput*			GetInputPtr()		= 0;

	public:
		virtual void				SetState(lfeFuncState state, lfeCallback value) = 0;
		virtual void				SetState(lfeBoolState state, bool value) = 0;
		virtual void				SetState(lfeIntState state, int value) = 0;
		virtual void				SetState(lfeStringState state, const std::wstring &value) = 0;

		virtual void				SetMsgProcCallback(lfeMsgProcCallback pMsgProcCallback) = 0;

	};

	//纹理
	class CLFTexture
	{
	public:
		virtual int						GetWidth(BOOL bOriginal=FALSE/*是否是原始大小*/)	= 0;

		virtual int						GetHeight(BOOL bOriginal=FALSE/*是否是原始大小*/)	= 0;

		virtual void					Release()											= 0;

		virtual DWORD*					Lock(BOOL bReadOnly = FALSE, 
			int nLeft	= 0, int nTop		= 0,
			int nWidth	= 0, int nHeight	= 0)	= 0;

		virtual void					Unlock()											= 0;
	};

	class CLFFont
	{
	public:
		virtual void					Draw(const std::wstring &strText, 
			pos_i &pt, DWORD Color = 0xff000000,float z = 0.5f)						= 0;

		virtual void					Release()									= 0;
	};

	//字体描述数据结构
	struct FontDes
	{
		FontDes()
		{
			_nWidth = 14;
			_nHeight = 14;
			_nStrokeSize = 0;
			_dwStrokeColor = 0xff000000;
			_dwStrokeFontColor = 0xffffffff;
			_bAntiAliased = FALSE;
		}
		UINT	_nWidth;			//字体宽度
		UINT	_nHeight;			//字体高度
		UINT	_nStrokeSize;		//字体描边宽度，0就是不描边
		DWORD	_dwStrokeColor;		//字体描边的颜色
		DWORD	_dwStrokeFontColor;	//描边字体颜色
		BOOL	_bAntiAliased;		//是否开启抗锯齿
	};

	class CLFResManager
	{
	public:
		virtual	CLFTexture*		LoadTexture(const std::wstring &strFileName, BOOL bMipmap) = 0;

		virtual CLFTexture*		LoadTextureFromMemory(const LPVOID pBuff, DWORD nBuffSize, BOOL bMipmap) = 0;

		virtual CLFFont*		LoadFont(const std::wstring &strFontName,const FontDes &ftDes)	= 0;
	};

	//渲染目标
	class CLFTarget
	{
	public:
		virtual CLFTexture*		GetTexture()	= 0;

		virtual void			Release()		= 0;
	};

	struct MouseState
	{
		int		m_nAbsX;
		int		m_nAbsY;
		int		m_nAbsZ;

		int		m_nRelX;
		int		m_nRelY;
		int		m_nRelZ;

		virtual BOOL buttonDown(MouseButtonID button) = 0;
	};

	class MouseListener
	{
	public:
		virtual bool MouseMoved(const MouseState * const pState) = 0;
		virtual bool MousePressed(const MouseState * const pState, MouseButtonID id) = 0;
		virtual bool MouseReleased(const MouseState * const pState, MouseButtonID id) = 0;
	};

	class KeyListener
	{
	public:
		virtual bool KeyPressed(const KeyCode &code) = 0;
		virtual bool KeyReleased(const KeyCode &code) = 0;
	};

	class CLFInput
	{
	public:
		enum Modifier
		{
			Shift = 0x0000001,
			Ctrl  = 0x0000010,
			Alt   = 0x0000100
		};

		virtual	MouseState * const GetMouseState() = 0;

		virtual void SetMouseEventCallback(MouseListener *pMouseListener) = 0;

		virtual void SetKeyboardEventCallback(KeyListener *pKeyListener) = 0;

		virtual BOOL IsKeyDown(KeyCode key) = 0;

		virtual const std::wstring& getAsString(KeyCode kc) = 0;

		virtual BOOL isModifierDown(Modifier mod) = 0;
	};

	//顶点定义
	struct Vertex
	{
		float			x, y;
		float			z;
		DWORD			color;
		float			tx, ty;
	};

	//Triple定义
	struct Triple
	{
		Triple();
		void setTriple(pos_f &pos0, pos_f &pos1, pos_f &pos2);
		void setZBuffer(float z);
		void setTextureCoordinate(pos_f &pos0, pos_f &pos1, pos_f &pos2);

		Vertex			v[3];
		CLFTexture*		pTex;
		int				blend;
	};

	//Quad定义
	struct Quad
	{
		Quad();
		void setQuadColor(DWORD color);
		void setQuad(pos_f &pos0, pos_f &pos1, pos_f &pos2, pos_f &pos3);
		void setQuadRect(rect_f *pRect);
		void setZBuffer(float z);
		void setTextureCoordinate(pos_f &pos0, pos_f &pos1, pos_f &pos2, pos_f &pos3);
		void setTextureRect(rect_f *pRect);

		Vertex			v[4];
		CLFTexture*		pTex;
		int				blend;
	};

	//图形接口
	class CLFVideo
	{
	public:
		virtual BOOL			BeginScene(CLFTarget* pTarget = NULL)				= 0;

		virtual void			EndScene()											= 0;

		virtual	void			Clear(DWORD color)									= 0;

		virtual CLFTexture*		CreateTexture(int nWidth, int nHeight)				= 0;

		virtual CLFTarget*		CreateTarget(int nWidth, int nHeight, bool nZbuffer)= 0;

		virtual void			SetClipping(int nX=0, int nY=0, int nW=0, int nH=0) = 0;

		virtual void			SetTransform(float x, float y, float dx, float dy, float rot, float hscale, float vscale) = 0;

		virtual	void			RenderLine(float fX1, float fY1, 
			float fX2, float fY2, 
			DWORD color, float fZ)		= 0;

		virtual void			RenderQuad(const Quad *quad)					= 0;

		virtual void			RenderTriple(const Triple *triple)				= 0;

		virtual Vertex*			StartBatch(lfePrim prim_type, CLFTexture *tex, int blend, int *max_prim) = 0;

		virtual void			FinishBatch(int nprim) = 0;

		virtual int				GetFps()					= 0;

		virtual float			GetDelta()					= 0;
	};
}