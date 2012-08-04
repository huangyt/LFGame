#include "LFEngine.h"
#include "LFGui.h"

using namespace LF;
using namespace LFG;

CLFEngine*				pEngine			= NULL;
CLFGui*					pGui			= NULL;

CLFVideo*				pVideo			= NULL;
CLFResManager*			pResManager		= NULL;
CLFInput*				pInput			= NULL;

CLFTexture*				pTexture		= NULL;
CLFTarget*				pTarget			= NULL;
CLFFont*				pFont			= NULL;

int nX = 0;
int nY = 0;

class InputEvent : public MouseListener, public KeyListener
{
public:
	virtual bool MouseMoved(const MouseState * const pState)
	{
		nX = pState->m_nAbsX;
		nY = pState->m_nAbsY;
		return true;
	}
	virtual bool MousePressed(const MouseState * const pState, MouseButtonID id)
	{
		return true;
	}
	virtual bool MouseReleased(const MouseState * const pState, MouseButtonID id)
	{
		return true;
	}

	virtual bool KeyPressed(const KeyCode &code)
	{
		pEngine->Log(LOG_DEBUG, L"%s pressed", pInput->getAsString(code).c_str());
		if(code == KC_RETURN)
		{
			if(pInput->isModifierDown(CLFInput::Ctrl))
			{
				pEngine->Log(LOG_DEBUG, L"press ctrl and enter...");
			}
			else
			{
				pEngine->Log(LOG_DEBUG, L"press enter...");
			}
		}

		return true;
	}

	virtual bool KeyReleased(const KeyCode &code)
	{
		return true;
	}
};

BOOL FrameFunc()
{
	float fDelta = pVideo->GetDelta();

	return TRUE;
}

BOOL RenderFunc()
{
	pVideo->BeginScene(pTarget);
	pVideo->Clear(0x00000000);

	pVideo->RenderLine(0,0,100,100,0xffff0000, 0.0f);

	pVideo->EndScene();

	pVideo->BeginScene();
	pVideo->Clear(0xffffffff);

	if(nX == 0)
		nX = pInput->GetMouseState()->m_nAbsX;
	if(nY == 0)
		nY = pInput->GetMouseState()->m_nAbsY;

	rect_f rect;
	rect.UpperLeftCorner.X = nX;
	rect.UpperLeftCorner.Y = nY;
	rect.LowerRightCorner.X = nX + pTarget->GetTexture()->GetWidth();
	rect.LowerRightCorner.Y = nY + pTarget->GetTexture()->GetHeight();

	Quad a;
	a.setQuadRect(&rect);
	a.setZBuffer(0.0f);
	a.pTex = pTarget->GetTexture();
	pVideo->RenderQuad(&a);

	if(pFont)
	{
		pFont->Draw(L"ÒýÇæ²âÊÔ", pos_i(0, 50), 0xffff0000, 0.1f);
		pFont->Draw(Format(L"FPS = %d\nDelta = %f", 
			pVideo->GetFps(),
			pVideo->GetDelta()),
			pos_i(0,0));
	}

	pVideo->EndScene();

	return TRUE;
}

BOOL ExitFunc()
{
	if(pFont)
		pFont->Release();
	if(pTexture)
		pTexture->Release();
	if(pTarget)
		pTarget->Release();
	return TRUE;
}

BOOL InitFunc()
{
	FontDes ftDes;
	ftDes._bAntiAliased = TRUE;
	ftDes._nStrokeSize = 0;
	ftDes._dwStrokeColor = 0xff000000;
	ftDes._dwStrokeFontColor = 0xffffff00;
	ftDes._nWidth = 40;
	ftDes._nHeight = 40;
	pFont = pResManager->LoadFont(L"msyh.ttf", ftDes);

	pTexture = pResManager->LoadTexture(L"26.dds", TRUE);

	pTarget = pVideo->CreateTarget(100,100,true);
	return TRUE;
}

BOOL MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return pGui->MsgProc(hwnd, msg, wParam, lParam);
}

INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int)
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	try
	{
		InputEvent event;

		pEngine = CLFEngine::CreateEngine();
		pGui = CLFGui::CreateGui();

		pEngine->SetState(LFE_INITFUNC, InitFunc);
		pEngine->SetState(LFE_FRAMEFUNC, FrameFunc);
		pEngine->SetState(LFE_RENDERFUNC, RenderFunc);
		pEngine->SetState(LFE_EXITFUNC, ExitFunc);
		//pEngine->SetState(LFE_VRESTOREFUNC, VideoRestore);

		pEngine->SetMsgProcCallback(MsgProc);

		pEngine->SetState(LFE_WINDOWED, TRUE);
		pEngine->SetState(LFE_SCREENWIDTH, 800);
		pEngine->SetState(LFE_SCREENHEIGHT, 600);

		if(pEngine->Initiate())
		{
			pResManager = pEngine->GetResManagerPtr();
			pVideo = pEngine->GetVideoPtr();
			pInput = pEngine->GetInputPtr();
			pInput->SetMouseEventCallback(&event);
			pInput->SetKeyboardEventCallback(&event);
			pEngine->Run();
		}

		pEngine->ShutDown();

		SAFE_RELEASE(pGui);
		SAFE_RELEASE(pEngine);
	}
	catch(...)
	{
		SAFE_RELEASE(pGui);
		SAFE_RELEASE(pEngine);
	}
	return 0;
}