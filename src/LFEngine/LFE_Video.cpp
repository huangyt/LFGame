#include "LFE_Video.h"

#include <Process.h>

#include "LFE_System.h"
#include "LFE_Target.h"
#include "LFE_Texture.h"
#include "LFE_Timer.h"
#include "LFE_Font.h"

namespace LF
{
	std::wstring GetModeFormatName(int nModeID)
	{
		switch(nModeID)
		{
		case 22:
			{
				return L"X8R8G8B8";
			}
		}
		return L"";
	}

	CLFE_Video::CLFE_Video()
	{
		m_bInitialize = FALSE;

		m_pD3D = NULL;
		m_pD3DDevice = NULL;

		m_nLFEFPS = LFEFPS_VSYNC;
		m_bZBuffer = TRUE;
		m_bTextureFilter = TRUE;
		m_pCurTarget = NULL;

		if(m_nLFEFPS>0) 
		{
			m_nFixedDelta=int(1000.0f/m_nLFEFPS);
		}
		else 
		{
			m_nFixedDelta=0;
		}
		m_fTime = 0.0f;
		m_fDeltaTime = 0.0f;
		m_nFPS = 0;

		m_pCurTexture = NULL;
		m_pVertexArray = NULL;
		m_pScreenSurf = NULL;
		m_pScreenDepth = NULL;
		m_pVB = NULL;
		m_pIB = NULL;
	}

	CLFE_Video::~CLFE_Video()
	{
		if(m_bInitialize)
		{
			Uninitialize();
		}
	}

	LPDIRECT3DDEVICE9 CLFE_Video::GetDevice()
	{
		return m_pD3DDevice;
	}

	BOOL CLFE_Video::Update()
	{
		static DWORD t0 = m_pTimer->GetTime();
		static DWORD t0fps = t0;
		static DWORD dt = 0;
		static int cfps = 0;

		do { dt=m_pTimer->GetTime() - t0; } while(dt < 1);

		if(dt >= m_nFixedDelta)
		{
			m_fDeltaTime=dt/1000.0f;
			if(m_fDeltaTime > 0.2f)
			{
				m_fDeltaTime = m_nFixedDelta ? m_nFixedDelta/1000.0f : 0.01f;
			}
			m_fTime += m_fDeltaTime;
			t0 = m_pTimer->GetTime();
			if(t0-t0fps <= 1000) 
			{
				cfps++;
			}
			else
			{
				m_nFPS=cfps; cfps=0; t0fps=t0;
			}

			if(pSystem->procFrameFunc)
			{
				if(!pSystem->procFrameFunc())
				{
					return FALSE;
				}
			}

			if(!pSystem->m_bMinimized && pSystem->procRenderFunc)
			{
				pSystem->procRenderFunc();
			}
			else
			{
				Sleep(1);
			}
		}
		else
		{
			if(m_nFixedDelta && dt+3 < m_nFixedDelta) 
			{
				Sleep(1);
			}
		}
		return TRUE;
	}

	int CLFE_Video::GetFps()
	{
		return m_nFPS;
	}

	float CLFE_Video::GetDelta()
	{
		return m_fDeltaTime;
	}

	void CLFE_Video::Clear(DWORD color)
	{
		if(m_pCurTarget)
		{
			if(m_pCurTarget->m_pDepth)
				m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0 );
			else
				m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0 );
		}
		else
		{
			if(m_bZBuffer)
				m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0 );
			else
				m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, color, 1.0f, 0 );
		}
	}

	void CLFE_Video::SetClipping(int nX, int nY, int nW, int nH)
	{
		D3DVIEWPORT9 vp;
		int nSrc_Width = 0, nSrc_Height = 0;

		if(m_pCurTarget && m_pCurTarget->GetTexture())
		{
			nSrc_Width = m_pCurTarget->GetTexture()->GetWidth();
			nSrc_Height = m_pCurTarget->GetTexture()->GetHeight();
		}
		else
		{
			nSrc_Width = pSystem->m_nScreenWidth;
			nSrc_Height = pSystem->m_nScreenHeight;
		}

		if(!nW) {
			vp.X = 0;
			vp.Y = 0;
			vp.Width = nSrc_Width;
			vp.Height = nSrc_Height;
		}
		else
		{
			if(nX<0) { nW+=nX; nX=0; }
			if(nY<0) { nH+=nY; nY=0; }

			if(nX+nW > nSrc_Width) nW=nSrc_Width-nX;
			if(nY+nH > nSrc_Height) nH=nSrc_Height-nY;

			vp.X=nX;
			vp.Y=nY;
			vp.Width=nW;
			vp.Height=nH;
		}

		vp.MinZ=0.0f;
		vp.MaxZ=1.0f;

		this->render_batch();
		m_pD3DDevice->SetViewport(&vp);

		D3DXMATRIX tmp;
		D3DXMatrixScaling(&m_matProj, 1.0f, -1.0f, 1.0f);
		D3DXMatrixTranslation(&tmp, -0.5f, +0.5f, 0.0f);
		D3DXMatrixMultiply(&m_matProj, &m_matProj, &tmp);
		D3DXMatrixOrthoOffCenterLH(&tmp, (float)vp.X, (float)(vp.X+vp.Width), -((float)(vp.Y+vp.Height)), -((float)vp.Y), vp.MinZ, vp.MaxZ);
		D3DXMatrixMultiply(&m_matProj, &m_matProj, &tmp);
		m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	}

	void CLFE_Video::SetTransform(float x, float y, float dx, float dy, float rot, float hscale, float vscale)
	{
		D3DXMATRIX tmp;

		if(vscale==0.0f) D3DXMatrixIdentity(&m_matView);
		else
		{
			D3DXMatrixTranslation(&m_matView, -x, -y, 0.0f);
			D3DXMatrixScaling(&tmp, hscale, vscale, 1.0f);
			D3DXMatrixMultiply(&m_matView, &m_matView, &tmp);
			D3DXMatrixRotationZ(&tmp, -rot);
			D3DXMatrixMultiply(&m_matView, &m_matView, &tmp);
			D3DXMatrixTranslation(&tmp, x+dx, y+dy, 0.0f);
			D3DXMatrixMultiply(&m_matView, &m_matView, &tmp);
		}

		render_batch();
		m_pD3DDevice->SetTransform(D3DTS_VIEW, &m_matView);
	}

	BOOL CLFE_Video::BeginScene(CLFTarget* pTarget /* = NULL */)
	{
		D3DDISPLAYMODE Mode;
		HRESULT hr = m_pD3DDevice->TestCooperativeLevel();
		if (hr == D3DERR_DEVICELOST) 
		{
			Sleep(1);
			return FALSE;
		}
		else if (hr == D3DERR_DEVICENOTRESET)
		{
			if(pSystem->m_bWindowed)
			{
				if(FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode)) || 
					Mode.Format==D3DFMT_UNKNOWN) 
				{
					pSystem->Log(LOG_ERROR, L"Can't determine desktop video mode");
					return FALSE;
				}
				m_d3dppW.BackBufferFormat = Mode.Format;
			}
			if(!VideoRestore())
			{
				return FALSE;
			}
		}

		if(m_pVertexArray)
		{
			pSystem->Log(LOG_ERROR, L"BeginScene: Scene is already being rendered");
			return FALSE;
		}

		LPDIRECT3DSURFACE9 pSurf=NULL, pDepth = NULL;

		CLFE_Target *pTargetTemp = reinterpret_cast<CLFE_Target*>(pTarget);
		if(pTargetTemp != m_pCurTarget)
		{
			if(pTargetTemp && pTargetTemp->m_pLFETexture)
			{
				pTargetTemp->m_pLFETexture->m_pTexture->GetSurfaceLevel(0, &pSurf);
				pDepth = pTargetTemp->m_pDepth;
			}
			else
			{
				pSurf = m_pScreenSurf;
				pDepth = m_pScreenDepth;
			}

			if(FAILED(m_pD3DDevice->SetDepthStencilSurface(pDepth)))
			{
				if(pTargetTemp) 
				{
					SAFE_RELEASE(pSurf);
				}
				pSystem->Log(LOG_ERROR, L"BeginScene: Can't set DepthStencilSurface");
				return FALSE;
			}

			if(FAILED(m_pD3DDevice->SetRenderTarget(0, pSurf)))
			{
				if(pTargetTemp) 
				{
					SAFE_RELEASE(pSurf);
				}
				pSystem->Log(LOG_ERROR, L"BeginScene: Can't set render target");
				return FALSE;
			}

			if(pTargetTemp)
			{
				SAFE_RELEASE(pSurf);

				if(pTargetTemp->m_pDepth)
				{
					m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
				}
				else
				{
					m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ); 
				}
				SetProjectionMatrix(pTargetTemp->m_nWidth, pTargetTemp->m_nHeight);
			}
			else
			{
				if(m_bZBuffer)
				{
					m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE ); 
				}
				else
				{
					m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ); 
				}
				SetProjectionMatrix(pSystem->m_nScreenWidth, pSystem->m_nScreenHeight);
			}

			m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
			D3DXMatrixIdentity(&m_matView);
			m_pD3DDevice->SetTransform(D3DTS_VIEW, &m_matView);

			m_pCurTarget = pTargetTemp;
		}

		m_pD3DDevice->BeginScene();

		if(m_pVB)
			m_pVB->Lock( 0, 0, (void**)&m_pVertexArray, D3DLOCK_DISCARD );

		return TRUE;
	}

	void CLFE_Video::EndScene()
	{
		render_batch(TRUE);

		m_pD3DDevice->EndScene();
		if(!m_pCurTarget)
		{
			m_pD3DDevice->Present( NULL, NULL, NULL, NULL );
		}
	}

	void CLFE_Video::RenderLine(float fX1, float fY1, float fX2, float fY2, DWORD color, float fZ)
	{
		if(m_pVertexArray)
		{
			if(m_CurPrimType != LFE_LINES || m_nPrims>=VERTEX_BUFFER_SIZE/LFE_LINES ||
				m_pCurTexture || m_CurBlendMode!=BLEND_DEFAULT)
			{
				render_batch();

				m_CurPrimType = LFE_LINES;
				if(m_CurBlendMode != BLEND_DEFAULT) 
				{
					SetBlendMode(BLEND_DEFAULT);
				}
				if(m_pCurTexture) 
				{
					m_pD3DDevice->SetTexture(0, 0); 
					m_pCurTexture = NULL; 
				}
			}

			int i=m_nPrims*LFE_LINES;
			m_pVertexArray[i].x		= fX1; 
			m_pVertexArray[i+1].x	= fX2;
			m_pVertexArray[i].y		= fY1; 
			m_pVertexArray[i+1].y	= fY2;
			m_pVertexArray[i].z     = m_pVertexArray[i+1].z = fZ;
			m_pVertexArray[i].color = m_pVertexArray[i+1].color = color;
			m_pVertexArray[i].tx    = m_pVertexArray[i+1].tx =
				m_pVertexArray[i].ty    = m_pVertexArray[i+1].ty = 0.0f;

			m_nPrims++;
		}
	}

	void CLFE_Video::RenderTriple(const Triple *triple)
	{
		if(m_pVertexArray)
		{
			CLFE_Texture *pTex = reinterpret_cast<CLFE_Texture*>(triple->pTex);
			if(m_CurPrimType!=LFE_TRIPLES || m_nPrims>=VERTEX_BUFFER_SIZE/LFE_TRIPLES || 
				m_pCurTexture!=pTex || m_CurBlendMode!=triple->blend)
			{
				render_batch();

				m_CurPrimType=LFE_TRIPLES;
				if(m_CurBlendMode != triple->blend) SetBlendMode(triple->blend);
				if( pTex != m_pCurTexture)
				{
					if(pTex)
					{
						m_pD3DDevice->SetTexture( 0, pTex->m_pTexture );
					}
					else
					{
						m_pD3DDevice->SetTexture( 0, NULL );
					}
					m_pCurTexture = pTex;
				}
			}

			memcpy(&m_pVertexArray[m_nPrims*LFE_TRIPLES], triple->v, sizeof(Vertex)*LFE_TRIPLES);
			m_nPrims++;
		}
	}

	void CLFE_Video::RenderQuad(const Quad *quad)
	{
		if(m_pVertexArray)
		{
			CLFE_Texture *pTex = reinterpret_cast<CLFE_Texture*>(quad->pTex);
			if(m_CurPrimType!=LFE_QUADS || m_nPrims>=VERTEX_BUFFER_SIZE/LFE_QUADS || 
				m_pCurTexture!=pTex || m_CurBlendMode!=quad->blend)
			{
				render_batch();

				m_CurPrimType=LFE_QUADS;
				if(m_CurBlendMode != quad->blend) SetBlendMode(quad->blend);
				if( pTex != m_pCurTexture)
				{
					if(pTex)
					{
						m_pD3DDevice->SetTexture( 0, pTex->m_pTexture );
					}
					else
					{
						m_pD3DDevice->SetTexture( 0, NULL );
					}
					m_pCurTexture = pTex;
				}
			}

			memcpy(&m_pVertexArray[m_nPrims*LFE_QUADS], quad->v, sizeof(Vertex)*LFE_QUADS);
			m_nPrims++;
		}
	}

	Vertex* CLFE_Video::StartBatch(lfePrim prim_type, CLFTexture *tex, int blend, int *max_prim)
	{
		CLFE_Texture *pTexture = dynamic_cast<CLFE_Texture*>(tex);
		if(m_pVertexArray && pTexture)
		{
			render_batch();
			m_CurPrimType = prim_type;
			if(m_CurBlendMode != blend) SetBlendMode(blend);
			if(tex != m_pCurTexture)
			{
				m_pD3DDevice->SetTexture( 0, pTexture->m_pTexture);
				m_pCurTexture = pTexture;
			}

			*max_prim=VERTEX_BUFFER_SIZE / prim_type;
			return m_pVertexArray;
		}
		else return 0;
	}

	void CLFE_Video::FinishBatch(int nprim)
	{
		m_nPrims = nprim;
	}

	void CLFE_Video::render_batch(BOOL bEndScene /* = FALSE */)
	{
		if(m_pVertexArray)
		{
			m_pVB->Unlock();

			if(m_nPrims)
			{
				switch(m_CurPrimType)
				{
				case LFE_QUADS:
					m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,m_nPrims<<2, 0, m_nPrims<<1);
					break;

				case LFE_TRIPLES:
					m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_nPrims);
					break;

				case LFE_LINES:
					m_pD3DDevice->DrawPrimitive(D3DPT_LINELIST, 0, m_nPrims);
					break;
				}

				m_nPrims=0;
			}

			if(bEndScene) 
			{
				m_pVertexArray = 0;
			}
			else 
			{
				m_pVB->Lock( 0, 0, (void**)&m_pVertexArray, D3DLOCK_DISCARD );
			}
		}
	}

	BOOL CLFE_Video::IsResolutionSupport(int nWidth, int nHeight)
	{
		if(!m_pD3D)
		{
			return FALSE;
		}

		UINT numModes = m_pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
		for (UINT i=0; i < numModes; i++) 
		{ 
			D3DDISPLAYMODE mode;  
			m_pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,D3DFMT_X8R8G8B8, i, &mode);                                       

			if ( mode.Width == nWidth && 
				mode.Height == nHeight ) 
			{  
				return TRUE;
			}
		}
		return FALSE;
	}

	void CLFE_Video::SetProjectionMatrix(int width, int height)
	{
		D3DXMATRIX tmp;
		D3DXMatrixScaling(&m_matProj, 1.0f, -1.0f, 1.0f);
		D3DXMatrixTranslation(&tmp, -0.5f, height+0.5f, 0.0f);
		D3DXMatrixMultiply(&m_matProj, &m_matProj, &tmp);
		D3DXMatrixOrthoOffCenterLH(&tmp, 0, (float)width, 0, (float)height, 0.0f, 1.0f);
		D3DXMatrixMultiply(&m_matProj, &m_matProj, &tmp);
	}

	BOOL CLFE_Video::Initialize()
	{
		m_bInitialize = TRUE;

		D3DDISPLAYMODE Mode;
		D3DADAPTER_IDENTIFIER9 AdID;
		// Init D3D
		m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		if(m_pD3D == NULL)
		{
			pSystem->Log(LOG_ERROR, L"无法创建D3D接口");
			return FALSE;
		}

		//Check Resolution Supprot
		if(!pSystem->m_bWindowed && 
			!this->IsResolutionSupport(pSystem->m_nScreenWidth, pSystem->m_nScreenHeight))
		{
			pSystem->Log(LOG_ERROR, L"%i x %i is not supprot in FullScreen Mode", 
				pSystem->m_nScreenWidth, pSystem->m_nScreenHeight);
			return FALSE;
		}

		// Get adapter info
		m_pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &AdID);
		pSystem->Log(LOG_DEBUG, L"D3D 驱动: %s",MBToUnicode(AdID.Driver).c_str());
		pSystem->Log(LOG_DEBUG, L"显卡: %s",MBToUnicode(AdID.Description).c_str());
		pSystem->Log(LOG_DEBUG, L"驱动版本: %d.%d.%d.%d",
			HIWORD(AdID.DriverVersion.HighPart),
			LOWORD(AdID.DriverVersion.HighPart),
			HIWORD(AdID.DriverVersion.LowPart),
			LOWORD(AdID.DriverVersion.LowPart));

		if(FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode)) || 
			Mode.Format==D3DFMT_UNKNOWN) 
		{
			pSystem->Log(LOG_ERROR, L"Can't determine desktop video mode");
			if(pSystem->m_bWindowed) 
			{
				return FALSE;
			}
		}

		// Set up Windowed presentation parameters
		ZeroMemory(&m_d3dppW, sizeof(m_d3dppW));

		m_d3dppW.BackBufferWidth	= pSystem->m_nScreenWidth;
		m_d3dppW.BackBufferHeight	= pSystem->m_nScreenHeight;
		m_d3dppW.BackBufferFormat	= Mode.Format;
		m_d3dppW.BackBufferCount	= 1;
		m_d3dppW.MultiSampleType	= D3DMULTISAMPLE_NONE;
		m_d3dppW.hDeviceWindow		= pSystem->m_hWnd;
		m_d3dppW.Windowed			= TRUE;
		m_d3dppW.SwapEffect			= D3DSWAPEFFECT_COPY;

		if(m_nLFEFPS == LFEFPS_VSYNC)
		{
			m_d3dppW.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
		else
		{
			m_d3dppW.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		if(m_bZBuffer)
		{
			m_d3dppW.EnableAutoDepthStencil     = TRUE; 
			m_d3dppW.AutoDepthStencilFormat     = D3DFMT_D24S8;
		}

		// Set up Full Screen presentation parameters
		ZeroMemory(&m_d3dppFS, sizeof(m_d3dppFS));

		m_d3dppFS.BackBufferWidth  = pSystem->m_nScreenWidth;
		m_d3dppFS.BackBufferHeight = pSystem->m_nScreenHeight;
		m_d3dppFS.BackBufferFormat = D3DFMT_X8R8G8B8;
		m_d3dppFS.BackBufferCount  = 1;
		m_d3dppFS.MultiSampleType  = D3DMULTISAMPLE_NONE;
		m_d3dppFS.hDeviceWindow    = pSystem->m_hWnd;
		m_d3dppFS.Windowed         = FALSE;
		m_d3dppFS.SwapEffect       = D3DSWAPEFFECT_FLIP;
		m_d3dppFS.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

		if(m_nLFEFPS == LFEFPS_VSYNC)
		{
			m_d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
		else
		{
			m_d3dppFS.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		if(m_bZBuffer)
		{
			m_d3dppFS.EnableAutoDepthStencil     = TRUE; 
			m_d3dppFS.AutoDepthStencilFormat     = D3DFMT_D24S8;
		}

		m_pD3dpp = pSystem->m_bWindowed ? &m_d3dppW : &m_d3dppFS;

		// Create D3D Device
		D3DCAPS9 caps;
		m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

		int vp = 0;
		if( (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) &&
			caps.VertexShaderVersion >= D3DVS_VERSION(1,1))
			vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

		if( FAILED( m_pD3D->CreateDevice( 
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
			pSystem->m_hWnd, 
			vp,
			m_pD3dpp, 
			&m_pD3DDevice ) ) )
		{
			m_pD3dpp->AutoDepthStencilFormat = D3DFMT_D16;
			if( FAILED(m_pD3D->CreateDevice(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				pSystem->m_hWnd,
				vp,
				m_pD3dpp,
				&m_pD3DDevice)) )
			{
				pSystem->Log(LOG_ERROR, L"Can't create D3D device");
				return FALSE;
			}
		}
		AdjustWindow();

		m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &Mode);
		pSystem->Log(LOG_DEBUG, L"显示模式: %d x %d x 32 Format: %s", 
			pSystem->m_nScreenWidth, pSystem->m_nScreenHeight, 
			GetModeFormatName(Mode.Format).c_str());

		m_pVertexArray = NULL;

		SetProjectionMatrix(pSystem->m_nScreenWidth, pSystem->m_nScreenHeight);
		D3DXMatrixIdentity(&m_matView);

		if(!init_lost()) 
		{
			return FALSE;
		}

		m_pTimer = new CLFE_Timer;
		if(m_pTimer)
		{
			m_pTimer->Play();
			pSystem->Log(LOG_DEBUG, L"高精度定时器创建成功");
		}
		else
		{
			pSystem->Log(LOG_ERROR, L"高精度计时器创建失败");
			return FALSE;
		}

		m_pFontManager = new CLFE_FontManager;
		if(m_pFontManager)
		{
			pSystem->Log(LOG_DEBUG, L"字体引擎创建成功");
		}
		else
		{
			pSystem->Log(LOG_ERROR, L"字体引擎创建失败");
		}

		this->Clear(0);

		return TRUE;
	}

	void CLFE_Video::Resize(int nWidth, int nHeight)
	{
		m_d3dppW.BackBufferWidth = nWidth;
		m_d3dppW.BackBufferHeight = nHeight;
		pSystem->m_nScreenWidth = nWidth;
		pSystem->m_nScreenHeight = nHeight;

		SetProjectionMatrix(nWidth, nHeight);
		VideoRestore();
	}

	void CLFE_Video::Uninitialize()
	{
		if(!m_bInitialize)
			return;
		m_bInitialize = FALSE;

		if(m_pFontManager)
		{
			SAFE_DELETE(m_pFontManager);
		}

		if(m_pTimer)
		{
			m_pTimer->Stop();
			SAFE_DELETE(m_pTimer);
		}

		for(CLFE_TexVec::iterator iter = m_TextureList.begin();
			iter != m_TextureList.end(); ++iter)
		{
			if(*iter)
			{
				SAFE_DELETE(*iter);
			}
		}

		for(CLFE_TarVec::iterator iter = m_TargetList.begin();
			iter != m_TargetList.end(); ++iter)
		{
			if(*iter)
			{
				SAFE_DELETE(*iter);
			}
		}

		if(m_pScreenSurf) 
		{ 
			SAFE_RELEASE(m_pScreenSurf);
		}

		if(m_pScreenDepth) 
		{ 
			SAFE_RELEASE(m_pScreenDepth);
		}

		if(m_pIB)
		{
			m_pD3DDevice->SetIndices(NULL);
			SAFE_RELEASE(m_pIB);
		}
		if(m_pVB)
		{
			if(m_pVertexArray) 
			{	
				m_pVB->Unlock(); 
				m_pVertexArray=0;	
			}
			m_pD3DDevice->SetStreamSource( 0, NULL, 0, sizeof(Vertex) );
			SAFE_RELEASE(m_pVB);
		}

		if(m_pD3D)
		{
			SAFE_RELEASE(m_pD3D);
		}

		if(m_pD3DDevice)
		{
			SAFE_RELEASE(m_pD3DDevice);
		}
	}

	BOOL CLFE_Video::VideoRestore()
	{
		if(m_pScreenSurf) 
		{
			SAFE_RELEASE(m_pScreenSurf);
		}
		if(m_pScreenDepth) 
		{
			SAFE_RELEASE(m_pScreenDepth);
		}

		for(CLFE_TarVec::iterator iter = m_TargetList.begin();
			iter != m_TargetList.end(); ++iter)
		{
			CLFE_Target *pTarget = *iter;
			if(pTarget && pTarget->m_pLFETexture)
			{
				pTarget->m_pLFETexture->m_pTexture->Release();

				if(pTarget->m_pDepth)
					pTarget->m_pDepth->Release();
			}
		}

		if(m_pIB)
		{
			m_pD3DDevice->SetIndices(NULL);
			SAFE_RELEASE(m_pIB);
		}
		if(m_pVB)
		{
			m_pD3DDevice->SetStreamSource( 0, NULL, 0, sizeof(Vertex) );
			SAFE_RELEASE(m_pVB);
		}

		m_pD3DDevice->Reset(m_pD3dpp);

		if(!init_lost()) 
		{
			return FALSE;
		}

		if(pSystem->procVideoRestoreFunc) 
		{
			return pSystem->procVideoRestoreFunc();
		}

		return TRUE;
	}

	BOOL CLFE_Video::init_lost()
	{
		m_pD3DDevice->GetRenderTarget(0, &m_pScreenSurf);
		m_pD3DDevice->GetDepthStencilSurface(&m_pScreenDepth);

		for(CLFE_TarVec::iterator iter = m_TargetList.begin();
			iter != m_TargetList.end(); ++iter)
		{
			CLFE_Target *pTarget = *iter;
			if(pTarget)
			{
				if(pTarget->m_pLFETexture && pTarget->m_pLFETexture->m_pTexture)
				{
					D3DXCreateTexture(m_pD3DDevice, pTarget->m_nWidth, pTarget->m_nHeight, 
						1, D3DUSAGE_RENDERTARGET,D3DFMT_A8R8G8B8/*m_pD3dpp->BackBufferFormat*/, 
						D3DPOOL_DEFAULT, &pTarget->m_pLFETexture->m_pTexture);
				}

				if(pTarget->m_pDepth)
				{
					m_pD3DDevice->CreateDepthStencilSurface(pTarget->m_nWidth, pTarget->m_nHeight,
						D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, FALSE, &pTarget->m_pDepth, NULL);
				}
			}
		}

		if( FAILED (m_pD3DDevice->CreateVertexBuffer(VERTEX_BUFFER_SIZE*sizeof(Vertex),
			D3DUSAGE_WRITEONLY,
			D3DFVF_HGEVERTEX,
			D3DPOOL_DEFAULT, &m_pVB, NULL)))
		{
			pSystem->Log(LOG_ERROR, L"Can't create D3D vertex buffer");
			return FALSE;
		}

		m_pD3DDevice->SetVertexShader(NULL);
		m_pD3DDevice->SetFVF(D3DFVF_HGEVERTEX);
		m_pD3DDevice->SetStreamSource( 0, m_pVB, 0, sizeof(Vertex) );

		if( FAILED( m_pD3DDevice->CreateIndexBuffer(VERTEX_BUFFER_SIZE*6/4*sizeof(WORD),
			D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT, &m_pIB, NULL) ) )
		{
			pSystem->Log(LOG_ERROR, L"Can't create D3D index buffer");
			return FALSE;
		}

		WORD* pIndices = NULL;
		WORD n = 0;
		if( FAILED( m_pIB->Lock( 0, 0, (VOID**)&pIndices, 0 ) ) )
		{
			pSystem->Log(LOG_ERROR, L"Can't lock D3D index buffer");
			return FALSE;
		}

		for(int i=0; i<VERTEX_BUFFER_SIZE/4; i++) {
			*pIndices++=n;
			*pIndices++=n+1;
			*pIndices++=n+2;
			*pIndices++=n+2;
			*pIndices++=n+3;
			*pIndices++=n;
			n+=4;
		}

		m_pIB->Unlock();
		m_pD3DDevice->SetIndices(m_pIB);

		// Set common render states
		m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		m_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_ALPHAREF,        0x01 );
		m_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		m_pD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		if(m_bTextureFilter)
		{
			m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
			m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		}
		else
		{
			m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT );
			m_pD3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		}

		m_nPrims = 0;
		m_CurPrimType = LFE_QUADS;
		m_CurBlendMode = BLEND_DEFAULT;

		if (FALSE == m_bZBuffer) {
			m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}

		m_pCurTexture = NULL;
		return TRUE;
	}

	BOOL CLFE_Video::SetWindowed(BOOL bWindowed)
	{
		if(m_pVertexArray)
		{
			return TRUE;
		}

		if(m_pD3DDevice && pSystem->m_bWindowed != bWindowed)
		{
			if(m_d3dppW.BackBufferFormat==D3DFMT_UNKNOWN || 
				m_d3dppFS.BackBufferFormat==D3DFMT_UNKNOWN) 
				return TRUE;

			if(pSystem->m_bWindowed) 
				GetWindowRect(pSystem->m_hWnd, &pSystem->m_rectW);

			pSystem->m_bWindowed=bWindowed;
			D3DPRESENT_PARAMETERS *d3dpp = NULL;
			if(pSystem->m_bWindowed) 
				d3dpp = &m_d3dppW;
			else 
				d3dpp = &m_d3dppFS;

			VideoRestore();
			AdjustWindow();
			return TRUE;
		}

		return FALSE;
	}

	void CLFE_Video::SetBlendMode(int nBlend)
	{
		if((nBlend & BLEND_ALPHABLEND) != (m_CurBlendMode & BLEND_ALPHABLEND))
		{
			if(nBlend & BLEND_ALPHABLEND) m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			else m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}

		if((nBlend & BLEND_ZWRITE) != (m_CurBlendMode & BLEND_ZWRITE))
		{
			if(nBlend & BLEND_ZWRITE) m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			else m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		}			

		if((nBlend & BLEND_COLORADD) != (m_CurBlendMode & BLEND_COLORADD))
		{
			if(nBlend & BLEND_COLORADD) m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
			else m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		}

		m_CurBlendMode = nBlend;
	}

	void CLFE_Video::AdjustWindow()
	{
		RECT *rc;
		LONG style;

		if(pSystem->m_bWindowed) 
		{
			rc = &pSystem->m_rectW; 
			style = pSystem->m_styleW; 
		}
		else  
		{
			rc = &pSystem->m_rectFS; 
			style = pSystem->m_styleFS; 
		}
		SetWindowLong(pSystem->m_hWnd, GWL_STYLE, style);

		style=GetWindowLong(pSystem->m_hWnd, GWL_EXSTYLE);
		if(pSystem->m_bWindowed)
		{
			SetWindowLong(pSystem->m_hWnd, GWL_EXSTYLE, style & (~WS_EX_TOPMOST));
			SetWindowPos(pSystem->m_hWnd, HWND_NOTOPMOST, 
				rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
		}
		else
		{
			SetWindowLong(pSystem->m_hWnd, GWL_EXSTYLE, style | WS_EX_TOPMOST);
			SetWindowPos(pSystem->m_hWnd, HWND_TOPMOST, 
				rc->left, rc->top, rc->right-rc->left, rc->bottom-rc->top, SWP_FRAMECHANGED);
		}
	}

	CLFTexture* CLFE_Video::CreateTexture(int nWidth, int nHeight)
	{
		CLFE_Texture* pTex = new CLFE_Texture;

		if(!pTex)
		{
			return NULL;
		}
		if( FAILED( D3DXCreateTexture( m_pD3DDevice, nWidth, nHeight,
			1,					// Mip levels
			0,					// Usage
			D3DFMT_A8R8G8B8,	// Format
			D3DPOOL_MANAGED,	// Memory pool
			&pTex->m_pTexture ) ) )
		{	
			pSystem->Log(LOG_ERROR, L"Can't create texture");
			SAFE_DELETE(pTex);
			return NULL;
		}
		pTex->m_nWidth = nWidth;
		pTex->m_nWidth = nHeight;

		m_TextureList.push_back(pTex);

		return reinterpret_cast<CLFTexture*>(pTex);
	}

	CLFTarget* CLFE_Video::CreateTarget(int nWidth, int nHeight, bool nZbuffer)
	{
		CLFE_Target* pTarget = new CLFE_Target;
		pTarget->m_pLFETexture = new CLFE_Texture;

		if(FAILED(D3DXCreateTexture(m_pD3DDevice, nWidth, nHeight, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8/*m_pD3dpp->BackBufferFormat*/, D3DPOOL_DEFAULT, &pTarget->m_pLFETexture->m_pTexture)))
		{
			pSystem->Log(LOG_ERROR, L"Can't create render target texture");
			SAFE_DELETE(pTarget);
			return NULL;
		}

		pTarget->m_nWidth = pTarget->m_pLFETexture->GetWidth();
		pTarget->m_nHeight = pTarget->m_pLFETexture->GetHeight();

		if(nZbuffer)
		{
			if(FAILED(m_pD3DDevice->CreateDepthStencilSurface(pTarget->m_nWidth, pTarget->m_nHeight,
				D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, FALSE, &pTarget->m_pDepth, NULL)))
			{   

				pSystem->Log(LOG_ERROR, L"Can't create render target depth buffer");
				SAFE_DELETE(pTarget);
				return NULL;
			}
		}

		m_TargetList.push_back(pTarget);

		return reinterpret_cast<CLFTarget*>(pTarget);
	}

	//////////////////////////////////////////////////////////////////////////
#define MACRO_CODE_ANGLE(n)\
	v[n].x = pos##n##.X;\
	v[n].y = pos##n##.Y;

#define MACRO_CODE_ANGLE_TEX(n)\
	v[n].tx = pTex ? pos##n##.X / static_cast<float>(pTex->GetWidth()) : pos##n##.X;\
	v[n].ty = pTex ? pos##n##.Y / static_cast<float>(pTex->GetHeight()) : pos##n##.Y;

	//! 默认构造
	Triple::Triple()
	{
		blend = BLEND_DEFAULT;
		pTex = NULL;
		for(int i=0;i<3;i++)
		{
			v[i].color = 0xFFFFFFFF;
			v[i].z = 0.5;
			v[i].x = v[i].y = 0.0;
			v[i].tx = v[i].ty = 0.0;
		}
	}

	//! 设置三角形的三个角坐标
	void Triple::setTriple(pos_f &pos0, pos_f &pos1, pos_f &pos2)
	{
		MACRO_CODE_ANGLE(0)
		MACRO_CODE_ANGLE(1)
		MACRO_CODE_ANGLE(2)
	}

	void Triple::setZBuffer(float z)
	{
		if(z >= 0.0f && z <= 1.0f)
		{
			for(int i=0;i<3;i++)
			{
				v[i].z = z;
			}
		}
	}

	//! 设置三角形三个角对应的纹理坐标
	void Triple::setTextureCoordinate(pos_f &pos0, pos_f &pos1, pos_f &pos2)
	{
		MACRO_CODE_ANGLE_TEX(0)
		MACRO_CODE_ANGLE_TEX(1)
		MACRO_CODE_ANGLE_TEX(2)
	}

	Quad::Quad()
	{
		blend = BLEND_DEFAULT;
		pTex = NULL;
		for(int i=0;i<4;i++)
		{
			v[i].color = 0xFFFFFFFF;
			v[i].z = 0.5;
			v[i].x = v[i].y = 0.0;
		}
		setTextureRect(NULL);
	}

	void Quad::setQuadColor(DWORD color)
	{
		for(int i=0;i<4;i++)
		{
			v[i].color = color;
		}
	}

	void Quad::setZBuffer(float z)
	{
		if(z >= 0.0f && z <= 1.0f)
		{
			for(int i=0;i<4;i++)
			{
				v[i].z = z;
			}
		}
	}

	//! 设置四边形四个角坐标
	void Quad::setQuad(pos_f &pos0, pos_f &pos1, pos_f &pos2, pos_f &pos3)
	{
		MACRO_CODE_ANGLE(0)
		MACRO_CODE_ANGLE(1)
		MACRO_CODE_ANGLE(2)
		MACRO_CODE_ANGLE(3)
	}

	//! 设置矩形
	void Quad::setQuadRect(rect_f *pRect)
	{
		if(!pRect)
		{
			for(int i=0;i<4;i++)
				v[i].x = v[i].y = v[i].z = 0.0;
		}
		else
		{
			v[0].x = pRect->UpperLeftCorner.X;
			v[0].y = pRect->UpperLeftCorner.Y;
			v[1].x = pRect->LowerRightCorner.X;
			v[1].y = v[0].y;
			v[2].x = v[1].x;
			v[2].y = pRect->LowerRightCorner.Y;
			v[3].x = v[0].x;
			v[3].y = v[2].y;
		}
	}

	//! 设置四个角的纹理坐标
	void Quad::setTextureCoordinate(pos_f &pos0, pos_f &pos1, pos_f &pos2, pos_f &pos3)
	{
		MACRO_CODE_ANGLE_TEX(0)
		MACRO_CODE_ANGLE_TEX(1)
		MACRO_CODE_ANGLE_TEX(2)
		MACRO_CODE_ANGLE_TEX(3)
	}

	//! 设置纹理矩形区域
	void Quad::setTextureRect(rect_f *pRect)
	{
		if(!pRect)
		{
			v[0].tx = v[3].tx = 0.0;
			v[1].tx = v[2].tx = 1.0;
			v[0].ty = v[1].ty = 0.0;
			v[2].ty = v[3].ty = 1.0;
		}
		else if(pTex)
		{
			v[0].tx = pRect->UpperLeftCorner.X / static_cast<float>(pTex->GetWidth());
			v[0].ty = pRect->UpperLeftCorner.Y / static_cast<float>(pTex->GetHeight());
			v[1].tx = pRect->LowerRightCorner.X / static_cast<float>(pTex->GetWidth());
			v[1].ty = v[0].ty;
			v[2].tx = v[1].tx;
			v[2].ty = pRect->LowerRightCorner.Y / static_cast<float>(pTex->GetHeight());
			v[3].tx = v[0].tx;
			v[3].ty = v[2].ty;
		}
	}

#undef MACRO_CODE_ANGLE
#undef MACRO_CODE_ANGLE_TEX
}