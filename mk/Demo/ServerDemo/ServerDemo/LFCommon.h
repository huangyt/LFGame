#pragma once

#include <signal.h>

#include "GlobalDef.h"
#include "SocketEngine.h"
#include "TimerEngine.h"
#include "AttemperEngine.h"
#include "AsynchronismEngine.h"
#include "DataBaseEngine.h"

#include "FormatString.h"
#include "LogEngine.h"
#include "SystemTime.h"
#include "Singleton.h"

#include "LFGameServer.h"

//////////////////////////////////////////////////////////////////////////
//ÄÚ´æÐ¹Â¶¼ì²â
#ifdef _DEBUG
#define DEBUG_CLIENTBLOCK   new(_CLIENT_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_CLIENTBLOCK
#endif

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define new DEBUG_CLIENTBLOCK
#endif
//////////////////////////////////////////////////////////////////////////

#define sLogEngine			CLogEngine::Instance()	
#define sGameServer			CLFGameServer::Instance()
#define sSystemTime			CSystemTime::Instance()