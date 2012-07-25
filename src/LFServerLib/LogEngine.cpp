#include "LogEngine.h"
#include "FormatString.h"

//////////////////////////////////////////////////////////////////////////

#define ID_LOG				1
#define LOG_BUFFER_SIZE		4096

struct LogInfo
{
	char		_srLog[LOG_BUFFER_SIZE];
	enLogLevel	_logLevel;
};

//////////////////////////////////////////////////////////////////////////

CLogEngine::CLogEngine()
{
	for (int i=0;i<EVENT_LEVEL_COUNT;i++) m_bLogShowFlag[i]=true;
	m_LogQueueService.SetQueueServiceSink(static_cast<CQueueServiceSink*>(this));
	m_LogQueueService.BeginService();
}

CLogEngine::~CLogEngine()
{
	m_LogQueueService.EndService();
}

void CLogEngine::Log(enLogLevel logLevel, std::string strLog, ...)
{
	LogInfo tempLogInfo;
	tempLogInfo._logLevel = logLevel;

	va_list cur_arg;
	va_start(cur_arg, strLog);
	_vsnprintf_s(tempLogInfo._srLog, LOG_BUFFER_SIZE, strLog.c_str(), cur_arg);
	va_end(cur_arg);

	m_LogQueueService.AddToQueue(ID_LOG, &tempLogInfo, sizeof(LogInfo));
}

void CLogEngine::ConfigShowFlag(enLogLevel LogLevel, bool bShow)
{
	ASSERT(LogLevel<EVENT_LEVEL_COUNT);
	if (LogLevel<EVENT_LEVEL_COUNT) m_bLogShowFlag[LogLevel]=bShow;
}

void CLogEngine::OnQueueServiceSink(WORD wIdentifier, void * pBuffer, WORD wDataSize)
{
	if(wIdentifier == ID_LOG && wDataSize == sizeof(LogInfo))
	{
		LogInfo *pTempInfo = static_cast<LogInfo*>(pBuffer);
		std::string strLogHead;
		switch(pTempInfo->_logLevel)
		{
		case Level_Normal:
			{
				strLogHead = "Normal";
				break;
			}
		case Level_Warning:
			{
				strLogHead = "Warning";
				break;
			}
		case Level_Exception:
			{
				strLogHead = "Exception";
				break;
			}
		case Level_Debug:
			{
				strLogHead = "Debug";
				break;
			}
		}

		std::string strLogTime;
		//获取时间
		SYSTEMTIME SystemTime;
		GetLocalTime(&SystemTime);
		strLogTime = std::move(strFormat("[%d/%02d/%02d %02d:%02d:%02d:%03d]",
					SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
					SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
					SystemTime.wMilliseconds));

		std::string strLog = std::move(strFormat("%s %s %s", strLogHead.c_str(), strLogTime.c_str(), pTempInfo->_srLog));

		if(m_bLogShowFlag[pTempInfo->_logLevel])
		{
			std::cout<<strLog<<std::endl;
		}
	}
}

//////////////////////////////////////////////////////////////////////////