#pragma once

#include "LFEngine.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace LF
{
	#define KEYBOARD_DX_BUFFERSIZE 17
	#define MOUSE_DX_BUFFERSIZE 128

	struct CLFE_MouseState : public MouseState
	{
		CLFE_MouseState();

		void	Clear();

		virtual BOOL buttonDown(MouseButtonID button);

		int			m_nWindowWidth;
		int			m_nWindowHeight;
		int			m_nButtons;
	};

	class CLFE_Input : public CLFInput
	{
	public:
		CLFE_Input(
			HWND hWnd, 
			BOOL bBuffered, 
			DWORD dwMouseCoopSetting, 
			DWORD dwKeyboardCoopSetting);
		virtual ~CLFE_Input();

		BOOL	Initialize();
		void	Capture();
		void	ResetWindowSize(int nWidth, int nHeight);

		virtual	MouseState * const GetMouseState();
		virtual void SetMouseEventCallback(MouseListener *pMouseListener);
		virtual void SetKeyboardEventCallback(KeyListener *pKeyListener);
		virtual BOOL IsKeyDown(KeyCode key);
		virtual BOOL isModifierDown(Modifier mod);
		virtual const std::wstring& getAsString(KeyCode kc);

	private:
		void CaptureMouse();
		void CaptureKeyboard();

		bool DoMouseClick(int mouseButton, DIDEVICEOBJECTDATA& di);
		void ReadKeyBuffered();
		void ReadKey();
		void ResetMousePos();

	private:
		IDirectInput8*			m_pDirectInput;
		IDirectInputDevice8*	m_pMouse;
		IDirectInputDevice8*	m_pKeyboard;

		CLFE_MouseState*		m_pMouseState;
		KeyListener*			m_pKeyListener;
		MouseListener*			m_pMouseListener;

		HWND					m_hWnd;
		BOOL					m_bBuffered;
		DWORD					m_dwMouseCoopSetting;
		DWORD					m_dwKeyboardCoopSetting;

		unsigned char			m_chKeyBuffer[256];
		unsigned int			m_Modifiers;
		std::wstring			m_GetString;
	};
}
