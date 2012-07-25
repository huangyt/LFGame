#include "LFE_Input.h"

#include "LFE_System.h"

namespace LF
{
	CLFE_MouseState::CLFE_MouseState()
		:m_nWindowHeight(50)
		,m_nWindowWidth(50)
	{
		Clear();
	}

	void CLFE_MouseState::Clear()
	{
		m_nAbsX = 0;
		m_nAbsY = 0;
		m_nAbsZ = 0;
		m_nRelX = 0;
		m_nRelY = 0;
		m_nRelZ = 0;
		m_nButtons = 0;
	}

	BOOL CLFE_MouseState::buttonDown(MouseButtonID button)
	{
		return ((m_nButtons & ( 1L << button )) == 0) ? FALSE : TRUE;
	}

	CLFE_Input::CLFE_Input(
		HWND hWnd, 
		BOOL bBuffered,
		DWORD dwMouseCoopSetting, 
		DWORD dwKeyboardCoopSetting)
		:m_hWnd(hWnd)
		,m_bBuffered(bBuffered)
		,m_dwMouseCoopSetting(dwKeyboardCoopSetting)
		,m_dwKeyboardCoopSetting(dwKeyboardCoopSetting)
	{
		m_pDirectInput = NULL;
		m_pKeyboard = NULL;
		m_pMouse = NULL;
		m_pMouseListener = NULL;
		m_pKeyListener = NULL;
		m_Modifiers = 0;

		memset(&m_chKeyBuffer, 0, 256);

		m_pMouseState = new CLFE_MouseState;

		if(0 == m_dwMouseCoopSetting)
		{
			m_dwMouseCoopSetting = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		}

		if(0 == m_dwKeyboardCoopSetting)
		{
			m_dwKeyboardCoopSetting = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
		}
	}

	CLFE_Input::~CLFE_Input()
	{
		if(m_pMouse)
		{
			m_pMouse->Unacquire();
		}

		if(m_pKeyboard)
		{
			m_pKeyboard->Unacquire();
		}

		SAFE_RELEASE(m_pKeyboard);
		SAFE_RELEASE(m_pMouse);
		SAFE_RELEASE(m_pDirectInput);

		SAFE_DELETE(m_pMouseState);
	}

	void CLFE_Input::ResetWindowSize(int nWidth, int nHeight)
	{
		if(m_pMouseState)
		{
			m_pMouseState->m_nWindowWidth = nWidth;
			m_pMouseState->m_nWindowHeight = nHeight;
		}
	}

	MouseState * const CLFE_Input::GetMouseState()
	{
		return reinterpret_cast<MouseState*>(m_pMouseState);
	}

	void CLFE_Input::SetMouseEventCallback(MouseListener *pMouseListener)
	{
		m_pMouseListener = pMouseListener;
	}

	void CLFE_Input::SetKeyboardEventCallback(KeyListener *pKeyListener)
	{
		m_pKeyListener = pKeyListener;
	}

	BOOL CLFE_Input::Initialize()
	{
		HRESULT hr;
		HINSTANCE hInst = GetModuleHandle(NULL);

		hr = DirectInput8Create( hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDirectInput, NULL );
		if(FAILED(hr))
		{
			pSystem->Log(LOG_ERROR, L"Not able to init DirectX8 Input!");
			return FALSE;
		}

		//mouse
		m_pMouseState->Clear();
		
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(m_hWnd, &point);
		m_pMouseState->m_nAbsX = point.x;
		m_pMouseState->m_nAbsY = point.y;

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = MOUSE_DX_BUFFERSIZE;

		hr = m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouse, NULL);
		if(FAILED(hr))
		{
			return FALSE;
		}

		hr = m_pMouse->SetDataFormat(&c_dfDIMouse2);
		if(FAILED(hr))
		{
			return FALSE;
		}

		hr = m_pMouse->SetCooperativeLevel(m_hWnd, m_dwMouseCoopSetting);
		if(FAILED(hr))
		{
			return FALSE;
		}

		hr = m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		if(FAILED(hr))
		{
			return FALSE;
		}

		hr = m_pMouse->Acquire();
		if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
		{
			return FALSE;
		}

		//keyboard
		hr = m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
		if(FAILED(hr))
		{
			return FALSE;
		}

		hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
		if(FAILED(hr))
		{
			return FALSE;
		}

		hr = m_pKeyboard->SetCooperativeLevel(m_hWnd, m_dwKeyboardCoopSetting);
		if(FAILED(hr))
		{
			return FALSE;
		}

		if(m_bBuffered)
		{
			DIPROPDWORD dipdw;
			dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
			dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			dipdw.diph.dwObj        = 0;
			dipdw.diph.dwHow        = DIPH_DEVICE;
			dipdw.dwData            = KEYBOARD_DX_BUFFERSIZE;

			hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
			if(FAILED(hr))
			{
				return FALSE;
			}
		}

		hr = m_pKeyboard->Acquire();
		if (FAILED(hr) && hr != DIERR_OTHERAPPHASPRIO)
		{
			return FALSE;
		}

		return TRUE;
	}

	void CLFE_Input::CaptureMouse()
	{
		if(NULL == m_pMouseState)
		{
			return;
		}

		m_pMouseState->m_nRelX = 0;
		m_pMouseState->m_nRelY = 0;
		m_pMouseState->m_nRelZ = 0;

		DIDEVICEOBJECTDATA diBuff[MOUSE_DX_BUFFERSIZE];
		DWORD entries = MOUSE_DX_BUFFERSIZE;

		HRESULT hr = m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0);
		if( hr != DI_OK )
		{
			hr = m_pMouse->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = m_pMouse->Acquire();

			hr = m_pMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0 );

			if( FAILED(hr) )
			{
				ResetMousePos();
				return;
			}
		}

		bool axesMoved = false;

		for(unsigned int i = 0; i < entries; ++i )
		{
			switch( diBuff[i].dwOfs )
			{
			case DIMOFS_BUTTON0:
				if(!DoMouseClick(0, diBuff[i])) return;
				break;
			case DIMOFS_BUTTON1:
				if(!DoMouseClick(1, diBuff[i])) return;
				break;
			case DIMOFS_BUTTON2:
				if(!DoMouseClick(2, diBuff[i])) return;
				break;
			case DIMOFS_BUTTON3:
				if(!DoMouseClick(3, diBuff[i])) return;
				break;
			case DIMOFS_BUTTON4:
				if(!DoMouseClick(4, diBuff[i])) return;
				break;	
			case DIMOFS_BUTTON5:
				if(!DoMouseClick(5, diBuff[i])) return;
				break;
			case DIMOFS_BUTTON6:
				if(!DoMouseClick(6, diBuff[i])) return;
				break;
			case DIMOFS_BUTTON7:
				if(!DoMouseClick(7, diBuff[i])) return;
				break;
			case DIMOFS_X:
				m_pMouseState->m_nRelX += diBuff[i].dwData;
				axesMoved = true;
				break;
			case DIMOFS_Y:
				m_pMouseState->m_nRelY += diBuff[i].dwData;
				axesMoved = true;
				break;
			case DIMOFS_Z:
				m_pMouseState->m_nRelZ += diBuff[i].dwData;
				axesMoved = true;
				break;
			default: break;
			} //end switch
		}//end for

		if( axesMoved )
		{
			if( m_dwMouseCoopSetting & DISCL_NONEXCLUSIVE )
			{
				//DirectInput provides us with meaningless values, so correct that
				POINT point;
				GetCursorPos(&point);
				ScreenToClient(m_hWnd, &point);
				m_pMouseState->m_nAbsX = point.x;
				m_pMouseState->m_nAbsY = point.y;
			}
			else
			{
				m_pMouseState->m_nAbsX +=  m_pMouseState->m_nRelX;
				m_pMouseState->m_nAbsY +=  m_pMouseState->m_nRelY;
			}
			m_pMouseState->m_nAbsZ +=  m_pMouseState->m_nRelZ;

			//Clip values to window
			if( m_pMouseState->m_nAbsX < 0 )
				m_pMouseState->m_nAbsX = 0;
			else if( m_pMouseState->m_nAbsX > m_pMouseState->m_nWindowWidth )
				m_pMouseState->m_nAbsX = m_pMouseState->m_nWindowWidth;
			if( m_pMouseState->m_nAbsY < 0 )
				m_pMouseState->m_nAbsY = 0;
			else if( m_pMouseState->m_nAbsY > m_pMouseState->m_nWindowHeight )
				m_pMouseState->m_nAbsY = m_pMouseState->m_nWindowHeight;

			//Do the move
			if( m_pMouseListener && m_bBuffered )
				m_pMouseListener->MouseMoved(m_pMouseState);
		}
	}

	void CLFE_Input::CaptureKeyboard()
	{
		if(m_bBuffered)
		{
			ReadKeyBuffered();
		}
		else
		{
			ReadKey();
		}
	}

	void CLFE_Input::Capture()
	{
		CaptureMouse();

		CaptureKeyboard();
	}

	bool CLFE_Input::DoMouseClick(int mouseButton, DIDEVICEOBJECTDATA& di)
	{
		if(di.dwData & 0x80)
		{
			m_pMouseState->m_nButtons |= 1 << mouseButton;
			if(m_pMouseListener && m_bBuffered)
			{
				return m_pMouseListener->MousePressed(m_pMouseState, (MouseButtonID)mouseButton);
			}
		}
		else
		{
			m_pMouseState->m_nButtons &= ~(1 << mouseButton);
			if(m_pMouseListener && m_bBuffered)
			{
				return m_pMouseListener->MouseReleased(m_pMouseState, (MouseButtonID)mouseButton);
			}
		}

		return true;
	}

	void CLFE_Input::ReadKeyBuffered()
	{
		DIDEVICEOBJECTDATA diBuff[KEYBOARD_DX_BUFFERSIZE];
		DWORD entries = KEYBOARD_DX_BUFFERSIZE;
		HRESULT hr;

		static bool verifyAfterAltTab = false;

		hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), diBuff, &entries, 0);
		if(hr != DI_OK)
		{
			hr = m_pKeyboard->Acquire();
			if (hr == E_ACCESSDENIED)
				verifyAfterAltTab = true;

			while(hr == DIERR_INPUTLOST)
				hr = m_pKeyboard->Acquire();

			return;
		}

		for(unsigned int i = 0; i < entries; ++i )
		{
			bool ret = true;
			KeyCode kc = (KeyCode)diBuff[ i ].dwOfs;

			m_chKeyBuffer[kc] = static_cast<unsigned char>(diBuff[ i ].dwData);

			if( diBuff[ i ].dwData & 0x80 )
			{
				//Turn on modifier
				if( kc == KC_LCONTROL || kc == KC_RCONTROL )
					m_Modifiers |= Ctrl;
				else if( kc == KC_LSHIFT || kc == KC_RSHIFT )
					m_Modifiers |= Shift;
				else if( kc == KC_LMENU || kc == KC_RMENU )
					m_Modifiers |= Alt;

				if(m_pKeyListener)
					ret = m_pKeyListener->KeyPressed(kc);
			}
			else
			{
				//Turn off modifier
				if( kc == KC_LCONTROL || kc == KC_RCONTROL )
					m_Modifiers &= ~Ctrl;
				else if( kc == KC_LSHIFT || kc == KC_RSHIFT )
					m_Modifiers &= ~Shift;
				else if( kc == KC_LMENU || kc == KC_RMENU )
					m_Modifiers &= ~Alt;

				//Fire off event
				if(m_pKeyListener)
					ret = m_pKeyListener->KeyReleased(kc);
			}

			if(ret == false)
				break;
		}

		if(verifyAfterAltTab)
		{
			bool ret = true;

			//Copy old buffer to temp location to compare against
			unsigned char keyBufferCopy[256];
			memcpy(keyBufferCopy, m_chKeyBuffer, 256);

			//Update new state
			ReadKey();

			for (unsigned i = 0; i < 256; i++)
			{
				if (keyBufferCopy[i] != m_chKeyBuffer[i])
				{
					if (m_pKeyListener)
					{
						if (m_chKeyBuffer[i])
							ret = m_pKeyListener->KeyPressed((KeyCode)i);
						else
							ret = m_pKeyListener->KeyReleased((KeyCode)i);
					}
				}

				if(ret == false)
					return;
			}

			verifyAfterAltTab = false;
		}
	}

	void CLFE_Input::ReadKey()
	{
		HRESULT  hr = m_pKeyboard->GetDeviceState(sizeof(m_chKeyBuffer), &m_chKeyBuffer);

		if( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED )
		{
			hr = m_pKeyboard->Acquire();
			if (hr != DIERR_OTHERAPPHASPRIO)
				m_pKeyboard->GetDeviceState(sizeof(m_chKeyBuffer), &m_chKeyBuffer);
		}

		//Set Shift, Ctrl, Alt
		m_Modifiers = 0;
		if( IsKeyDown(KC_LCONTROL) || IsKeyDown(KC_RCONTROL) )
			m_Modifiers |= Ctrl;
		if( IsKeyDown(KC_LSHIFT) || IsKeyDown(KC_RSHIFT) )
			m_Modifiers |= Shift;
		if( IsKeyDown(KC_LMENU) || IsKeyDown(KC_RMENU) )
			m_Modifiers |= Alt;
	}

	void CLFE_Input::ResetMousePos()
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(m_hWnd, &point);
		m_pMouseState->m_nAbsX = point.x;
		m_pMouseState->m_nAbsY = point.y;

		//Clip values to window
		if( m_pMouseState->m_nAbsX < 0 )
			m_pMouseState->m_nAbsX = 0;
		else if( m_pMouseState->m_nAbsX > m_pMouseState->m_nWindowWidth )
			m_pMouseState->m_nAbsX = m_pMouseState->m_nWindowWidth;
		if( m_pMouseState->m_nAbsY < 0 )
			m_pMouseState->m_nAbsY = 0;
		else if( m_pMouseState->m_nAbsY > m_pMouseState->m_nWindowHeight )
			m_pMouseState->m_nAbsY = m_pMouseState->m_nWindowHeight;

		static int nTempX = m_pMouseState->m_nAbsX;
		static int nTempY = m_pMouseState->m_nAbsY;
		if(nTempX != m_pMouseState->m_nAbsX || nTempY != m_pMouseState->m_nAbsY)
		{
			nTempX = m_pMouseState->m_nAbsX;
			nTempY = m_pMouseState->m_nAbsY;
			//Do the move
			if( m_pMouseListener && m_bBuffered )
				m_pMouseListener->MouseMoved(m_pMouseState);
		}
	}

	BOOL CLFE_Input::IsKeyDown(KeyCode key)
	{
		return (m_chKeyBuffer[key] & 0x80) != 0;
	}

	BOOL CLFE_Input::isModifierDown(Modifier mod)
	{
		return (m_Modifiers & mod);
	}

	const std::wstring& CLFE_Input::getAsString(KeyCode kc)
	{
		DIPROPSTRING prop;
		prop.diph.dwSize = sizeof(DIPROPSTRING);
		prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		prop.diph.dwObj = static_cast<DWORD>(kc);
		prop.diph.dwHow = DIPH_BYOFFSET;

		if (SUCCEEDED(m_pKeyboard->GetProperty(DIPROP_KEYNAME, &prop.diph)))
		{
			return m_GetString.assign(prop.wsz);
		}

		m_GetString = std::move(Format(L"Key_%i", (int)kc));
		return m_GetString;
	}
}
