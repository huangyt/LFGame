#pragma once

#include "LFEngine.h"
#include <d3dx9.h>

#define VERTEX_BUFFER_SIZE	4000
#define D3DFVF_HGEVERTEX	D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1

namespace LF
{
	class CLFE_Timer;
	class CLFE_Target;
	class CLFE_Texture;
	class CLFE_FontManager;

	typedef std::vector<CLFE_Texture*>	CLFE_TexVec;
	typedef std::vector<CLFE_Target*>	CLFE_TarVec;

	class CLFE_Video : public CLFVideo
	{
		friend class CLFE_System;
	public:
		CLFE_Video();
		~CLFE_Video();

		virtual BOOL BeginScene(CLFTarget* pTarget = NULL);
		virtual void EndScene();

		virtual void Clear(DWORD color);

		virtual CLFTexture* CreateTexture(int nWidth, int nHeight);
		virtual CLFTarget* CreateTarget(int nWidth, int nHeight, bool nZbuffer);

		virtual void SetClipping(int nX=0, int nY=0, int nW=0, int nH=0);
		virtual void SetTransform(float x, float y, float dx, float dy, float rot, float hscale, float vscale);

		virtual void RenderLine(float fX1, float fY1, float fX2, float fY2, DWORD color, float fZ);
		virtual void RenderQuad(const Quad *quad);
		virtual void RenderTriple(const Triple *triple);

		virtual Vertex*	StartBatch(lfePrim prim_type, CLFTexture *tex, int blend, int *max_prim);
		virtual void FinishBatch(int nprim);

		virtual int GetFps();
		virtual float GetDelta();

		void Resize(int nWidth, int nHeight);

	private:
		BOOL Initialize();
		void Uninitialize();
		BOOL Render();

		BOOL init_lost();
		BOOL VideoRestore();

		BOOL IsResolutionSupport(int nWidth, int nHeight);
		void SetProjectionMatrix(int width, int height);
		void AdjustWindow();

		void render_batch(BOOL bEndScene = FALSE);
		void SetBlendMode(int nBlend);
		BOOL SetWindowed(BOOL bWindowed);

	private:
		BOOL					m_bInitialize;

		IDirect3D9*				m_pD3D;
		IDirect3DDevice9*		m_pD3DDevice;

		int						m_nLFEFPS;
		BOOL					m_bZBuffer;
		BOOL					m_bTextureFilter;
		D3DPRESENT_PARAMETERS*	m_pD3dpp;

		IDirect3DSurface9*		m_pScreenSurf;
		IDirect3DSurface9*		m_pScreenDepth;
		IDirect3DVertexBuffer9*	m_pVB;
		IDirect3DIndexBuffer9*	m_pIB;

		D3DXMATRIX				m_matView;
		D3DXMATRIX				m_matProj;

		//窗口模式数据
		D3DPRESENT_PARAMETERS	m_d3dppW;

		//全屏模式数据
		D3DPRESENT_PARAMETERS	m_d3dppFS;

		float					m_fTime;
		float					m_fDeltaTime;
		DWORD					m_nFixedDelta;
		int						m_nFPS;

		int						m_nPrims;
		int						m_CurBlendMode;
		lfePrim					m_CurPrimType;
		CLFE_Texture*			m_pCurTexture;
		CLFE_Target*			m_pCurTarget;
		Vertex*					m_pVertexArray;

		CLFE_Timer*				m_pTimer;
	public:
		//共享数据
		CLFE_TexVec				m_TextureList;
		CLFE_TarVec				m_TargetList;

		CLFE_FontManager*		m_pFontManager;

		//共享函数
		LPDIRECT3DDEVICE9		GetDevice();
	};
}