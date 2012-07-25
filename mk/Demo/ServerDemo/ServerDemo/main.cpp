#include "LFCommon.h"

bool stopEvent = false;

void UnhookSignals();
void HookSignals();

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	sGameServer.BegineService();

	HookSignals();

	while(!stopEvent)
	{
		Sleep(1);
	}

	sGameServer.EndService();

	UnhookSignals();

	return 0;
}

void OnSignal(int s)
{
	switch (s)
	{
	case SIGINT:
	case SIGTERM:
	case SIGBREAK:
		stopEvent = true;
		Sleep(1000);
		break;
	}

	signal(s, OnSignal);
}

void HookSignals()
{
	signal(SIGINT, OnSignal);
	signal(SIGTERM, OnSignal);
	signal(SIGBREAK, OnSignal);
}

void UnhookSignals()
{
	signal(SIGINT, 0);
	signal(SIGTERM, 0);
	signal(SIGBREAK, 0);
}