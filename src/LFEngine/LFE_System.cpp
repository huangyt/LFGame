#include "LFE_System.h"

#include "LFE_ResManager.h"
#include "LFE_Timer.h"
#include "LFE_Input.h"
#include "LFE_Video.h"

#include <process.h>

namespace LF
{
#define WINDOW_CLASS_NAME		L"LFE__WNDCLASS"

	int				nRef = 0;
	CLFE_System*	pSystem = NULL;

	CLFEngine* CLFEngine::CreateEngine()
	{
		return reinterpret_cast<CLFEngine*>(CLFE_System::_Interface_Get());	
	}

	CLFVideo* CLFE_System::GetVideoPtr()
	{
		return reinterpret_cast<CLFVideo*>(m_pVideo);
	}

	CLFResManager* CLFE_System::GetResManagerPtr()
	{
		return reinterpret_cast<CLFResManager*>(m_pResManager);
	}

	CLFInput* CLFE_System::GetInputPtr()
	{
		return reinterpret_cast<CLFInput*>(m_pInput);
	}

	CLFE_System* CLFE_System::_Interface_Get()
	{
		if(!pSystem) 
		{
			pSystem=new CLFE_System;
		}

		++nRef;

		return pSystem;
	}

	void CLFE_System::Release()
	{
		--nRef;

		if(!nRef)
		{
			if(pSystem->m_hWnd)
			{
				pSystem->ShutDown();
			}
			SAFE_DELETE(pSystem);
		}
	}

	CLFE_System::CLFE_System()
	{
		m_hWnd = NULL;
		m_hInstance = GetModuleHandle(NULL);

		m_strWinTitle = L"LFEngine Title";
		m_bWindowed = TRUE;
		m_bActive = FALSE;
		m_bMinimized = FALSE;
		m_nScreenWidth = 800;
		m_nScreenHeight = 600;

		m_bRenderRun = FALSE;
		m_hRenderThread = NULL;

		procInitFunc			= NULL;
		procExitFunc			= NULL;
		procFrameFunc			= NULL;
		procRenderFunc			= NULL;
		prceFocusLostFunc		= NULL;
		procFocusGainFunc		= NULL;
		procVideoRestoreFunc	= NULL;

		m_pVideo		= NULL;
		m_pInput		= NULL;
		m_pResManager	= NULL;

		WCHAR pAppPath[_MAX_PATH] = {0};
		GetModuleFileName(GetModuleHandle(NULL), pAppPath, sizeof(pAppPath));

		for(int i=wcslen(pAppPath)-1; i>0; i--) 
		{
			if(pAppPath[i]=='\\') 
			{
				pAppPath[i+1]=0;
				break;
			}
		}

		m_strAppPath = pAppPath;
	}

	BOOL CLFE_System::Initiate()
	{
		OSVERSIONINFO	os_ver;
		MEMORYSTATUS	mem_st;
		//Log system info
		Log(LOG_DEBUG, L"LF ���濪ʼ����...");

		Log(LOG_DEBUG, L"��������: %s", m_strWinTitle.c_str());
		os_ver.dwOSVersionInfoSize=sizeof(os_ver);
		GetVersionEx(&os_ver);
		Log(LOG_DEBUG, L"ϵͳ�汾: Windows %ld.%ld.%ld",os_ver.dwMajorVersion,
			os_ver.dwMinorVersion,os_ver.dwBuildNumber);

		GlobalMemoryStatus(&mem_st);
		Log(LOG_DEBUG, L"�ڴ�ʹ����: %ldK total, %ldK free",
			mem_st.dwTotalPhys/1024L,mem_st.dwAvailPhys/1024L);

		// Register window class
		WNDCLASS winclass;
		winclass.style			= CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		winclass.lpfnWndProc	= WindowProc;
		winclass.cbClsExtra		= 0;
		winclass.cbWndExtra		= 0;
		winclass.hInstance		= m_hInstance;
		winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
		winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
		winclass.lpszMenuName	= NULL; 
		winclass.lpszClassName	= WINDOW_CLASS_NAME;
		if(!m_strIcon.empty())
		{
			winclass.hIcon = LoadIcon(m_hInstance, m_strIcon.c_str());
		}
		else
		{
			winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		}

		if (!RegisterClass(&winclass))
		{
			Log(LOG_ERROR, L"�޷�ע�ᴰ��");
			return FALSE;
		}
		
		// Create window
		int nWidth, nHeight;

		nWidth = m_nScreenWidth + GetSystemMetrics(SM_CXFIXEDFRAME)*2;
		nHeight = m_nScreenHeight + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + 
			GetSystemMetrics(SM_CYCAPTION);

		m_rectW.left = (GetSystemMetrics(SM_CXSCREEN)-nWidth)/2;
		m_rectW.top = (GetSystemMetrics(SM_CYSCREEN)-nHeight)/2;
		m_rectW.right = m_rectW.left+nWidth;
		m_rectW.bottom = m_rectW.top+nHeight;
		m_styleW = WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE;

		m_rectFS.left = 0;
		m_rectFS.top = 0;
		m_rectFS.right = m_nScreenWidth;
		m_rectFS.bottom = m_nScreenHeight;
		m_styleFS = WS_POPUP|WS_VISIBLE;

		if(m_bWindowed)
			m_hWnd = CreateWindowEx(0, WINDOW_CLASS_NAME, m_strWinTitle.c_str(), m_styleW,
			m_rectW.left, m_rectW.top, m_rectW.right-m_rectW.left, m_rectW.bottom-m_rectW.top,
			NULL, NULL, m_hInstance, NULL);
		else
			m_hWnd = CreateWindowEx(WS_EX_TOPMOST, WINDOW_CLASS_NAME, m_strWinTitle.c_str(), 
			m_styleFS, 0, 0, 0, 0,
			NULL, NULL, m_hInstance, NULL);

		if(!m_hWnd)
		{
			Log(LOG_ERROR, L"�޷���������");
			return FALSE;
		}

		m_pResManager  = new CLFE_ResManager;
		if(m_pResManager)
		{
			Log(LOG_DEBUG, L"��Դ�����������ɹ�");
		}

		m_pInput = new CLFE_Input(m_hWnd, TRUE, 0, 0);
		if(m_pInput)
		{
			m_pInput->Initialize();
			m_pInput->ResetWindowSize(m_nScreenWidth, m_nScreenHeight);
			Log(LOG_DEBUG, L"����ϵͳ�����ɹ�");
		}

 		Log(LOG_DEBUG, L"��ʼ������Ⱦ����");
 		m_pVideo = new CLFE_Video;
 		if(m_pVideo)
 		{
 			Log(LOG_DEBUG, L"��Ⱦ������سɹ�...");
 		}

		if(!m_pResManager || !m_pInput || !m_pVideo)
		{
			return FALSE;
		}

		m_bRenderRun = TRUE;
		m_hRenderThread = (HANDLE)_beginthreadex(NULL,0,RenderThreadFunction,this,0,NULL);

		return TRUE;
	}

	unsigned __stdcall CLFE_System::RenderThreadFunction(LPVOID pThreadData)
	{
		CLFE_System *pSystem =(CLFE_System*)pThreadData;

		if(!pSystem->m_pVideo->Initialize())
		{
			pSystem->Log(LOG_ERROR, L"��ʼ��DirectXʧ��");
			return -1;
		}
		ShowWindow(pSystem->m_hWnd, SW_SHOW);

		if(pSystem->procInitFunc)
		{
			pSystem->procInitFunc();
		}

		while (pSystem->m_bRenderRun)
		{
			pSystem->m_pVideo->Update();
		}

		if(pSystem->procExitFunc)
		{
			pSystem->procExitFunc();
		}

		pSystem->m_pVideo->Uninitialize();

		return 0;
	}

	void CLFE_System::ShutDown()
	{
		if(m_hWnd)
		{
			ShowWindow(m_hWnd, SW_HIDE);
		}

		m_bRenderRun = FALSE;
		WaitForSingleObject(m_hRenderThread, INFINITE);
		CloseHandle(m_hRenderThread);

 		if(m_pVideo)
 		{
 			SAFE_DELETE(m_pVideo);
 		}

		if(m_pResManager)
		{
			SAFE_DELETE(m_pResManager);
		}

		if(m_pInput)
		{
			SAFE_DELETE(m_pInput);
		}

		if(m_hWnd)
		{
			DestroyWindow(m_hWnd);
			m_hWnd = NULL;
		}

		if(m_hInstance)
		{
			UnregisterClass(WINDOW_CLASS_NAME, m_hInstance);
		}
	}

	BOOL CLFE_System::Run()
	{
		MSG msg;

		if(!m_hWnd)
		{
			Log(LOG_ERROR, L"Run: Initiate wasn't called");
			return FALSE;
		}

		m_bActive = TRUE;
		for(;;)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{ 
				if (msg.message == WM_QUIT)	
				{
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if(m_pInput)
				{
					m_pInput->Capture();
				}
				Sleep(1);
			}
		}
		m_bActive = FALSE;

		return TRUE;
	}

	void CLFE_System::SetState(lfeFuncState state, lfeCallback value)
	{
		switch(state)
		{
		case LFE_FRAMEFUNC:
			{
				procFrameFunc = value;
				break;
			}
		case LFE_RENDERFUNC:
			{
				procRenderFunc = value;
				break;
			}
		case LFE_VRESTOREFUNC:
			{
				procVideoRestoreFunc = value;
				break;
			}
		case LFE_EXITFUNC:
			{
				procExitFunc = value;
				break;
			}
		case LFE_INITFUNC:
			{
				procInitFunc = value;
				break;
			}
		}
	}

	void CLFE_System::SetState(lfeBoolState state, bool value)
	{
		switch(state)
		{
		case LFE_WINDOWED:
			{
				if(!m_pVideo || !m_pVideo->SetWindowed(value))
				{
					m_bWindowed = value;
				}
				break;
			}
		case LFE_ZBUFFER:
			{
				break;
			}
		}
	}

	void CLFE_System::SetState(lfeIntState state, int value)
	{
		switch(state)
		{
		case LFE_SCREENWIDTH:
			{
				if(!m_pVideo) m_nScreenWidth=value;
				break;
			}
		case LFE_SCREENHEIGHT:
			{
				if(!m_pVideo) m_nScreenHeight=value;
				break;
			}
		}
	}

	void CLFE_System::SetState(lfeStringState state, const std::wstring &value)
	{
		switch(state)
		{
		case LFE_TITLE:
			{
				m_strWinTitle = value;
				if(m_hWnd)
				{
					SetWindowText(m_hWnd, m_strWinTitle.c_str());
				}
				break;
			}
		}
	}

	std::wstring CLFE_System::GetErrorMessage()
	{
		return m_strError;
	}

	void CLFE_System::FocusChange(BOOL bAct)
	{
		m_bActive=bAct;

		if(m_bActive)
		{
			if(procFocusGainFunc)
			{
				procFocusGainFunc();
			}
		}
		else
		{
			if(prceFocusLostFunc)
			{
				prceFocusLostFunc();
			}
		}
	}

	LRESULT CALLBACK CLFE_System::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bActivating = FALSE;
		switch(msg)
		{
		case WM_CREATE:
			{
				return FALSE;
			}
		case WM_PAINT:
			{
				break;
			}
		case WM_DESTROY:
			{
				PostQuitMessage(0);
				return FALSE;
			}
		case WM_ACTIVATE:
			{
				pSystem->m_bMinimized = HIWORD(wParam);

				bActivating = (LOWORD(wParam) != WA_INACTIVE) && (HIWORD(wParam) == 0);
				if(pSystem->m_pVideo && pSystem->m_pVideo->m_pD3D && pSystem->m_bActive != bActivating) 
				{
					pSystem->FocusChange(bActivating);
				}
				return FALSE;
			}
		case WM_SYSCOMMAND:
			{
				break;
			}
		default:
			break;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}