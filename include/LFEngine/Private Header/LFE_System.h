#pragma once

#include "LFEngine.h"

namespace LF
{
	class CLFE_Video;
	class CLFE_Input;
	class CLFE_ResManager;

	class CLFE_System : public CLFEngine
	{
	public:
		virtual BOOL			Initiate();

		virtual BOOL			Run();

		virtual void			ShutDown();

		virtual void			Release();

		virtual std::wstring	GetErrorMessage();

		virtual CLFVideo*		GetVideoPtr();

		virtual CLFResManager*	GetResManagerPtr();

		virtual CLFInput*		GetInputPtr();

	public:
		virtual void			SetState(lfeFuncState state, lfeCallback value);
		virtual void			SetState(lfeBoolState state, bool value);
		virtual void			SetState(lfeIntState state, int value);
		virtual void			SetState(lfeStringState state, const std::wstring &value);

	public:
		static  CLFE_System*	_Interface_Get();

		void					FocusChange(BOOL bAct);
	public:
		HWND			m_hWnd;
		HINSTANCE		m_hInstance;

		std::wstring	m_strAppPath;
		std::wstring	m_strIcon;
		std::wstring	m_strWinTitle;
		std::wstring	m_strError;
		BOOL			m_bWindowed;
		BOOL			m_bActive;
		BOOL			m_bMinimized;
		int				m_nScreenWidth;
		int				m_nScreenHeight;

		//线程
		volatile BOOL	m_bRenderRun;
		HANDLE			m_hRenderThread;
		static unsigned __stdcall RenderThreadFunction(LPVOID pThreadData);

		//外调函数
		lfeCallback		procInitFunc;
		lfeCallback		procFrameFunc;
		lfeCallback		procRenderFunc;
		lfeCallback		prceFocusLostFunc;
		lfeCallback		procFocusGainFunc;
		lfeCallback		procVideoRestoreFunc;
		lfeCallback		procExitFunc;
		//窗口模式信息
		RECT			m_rectW;
		LONG			m_styleW;

		//全屏模式数据
		RECT			m_rectFS;
		LONG			m_styleFS;

		//内部指针
		CLFE_Video*			m_pVideo;
		CLFE_Input*			m_pInput;
		CLFE_ResManager*	m_pResManager;
	private:
		CLFE_System();
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};

	extern CLFE_System*		pSystem;
}