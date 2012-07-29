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

		virtual void			SetMsgProcCallback(lfeMsgProcCallback pMsgProcCallback);

	public:
		static  CLFE_System*	_Interface_Get();

		void					FocusChange(BOOL bAct);
	public:
		HWND				m_hWnd;
		HINSTANCE			m_hInstance;

		std::wstring		m_strAppPath;
		std::wstring		m_strIcon;
		std::wstring		m_strWinTitle;
		std::wstring		m_strError;
		BOOL				m_bWindowed;
		BOOL				m_bActive;
		BOOL				m_bMinimized;
		int					m_nScreenWidth;
		int					m_nScreenHeight;

		//�߳�
		volatile BOOL		m_bRenderRun;
		HANDLE				m_hRenderThread;
		static unsigned __stdcall RenderThreadFunction(LPVOID pThreadData);

		//�������
		lfeCallback			m_procInitFunc;
		lfeCallback			m_procFrameFunc;
		lfeCallback			m_procRenderFunc;
		lfeCallback			m_prceFocusLostFunc;
		lfeCallback			m_procFocusGainFunc;
		lfeCallback			m_procVideoRestoreFunc;
		lfeCallback			m_procExitFunc;

		lfeMsgProcCallback	m_lpMsgProcCallback;

		//����ģʽ��Ϣ
		RECT				m_rectW;
		LONG				m_styleW;

		//ȫ��ģʽ����
		RECT				m_rectFS;
		LONG				m_styleFS;

		//�ڲ�ָ��
		CLFE_Video*			m_pVideo;
		CLFE_Input*			m_pInput;
		CLFE_ResManager*	m_pResManager;
	private:
		CLFE_System();
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};

	extern CLFE_System*		pSystem;
}