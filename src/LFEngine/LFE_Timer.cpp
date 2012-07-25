#include "LFE_Timer.h"

#include "LFE_System.h"

namespace LF
{
	__int64 CLFE_Timer::m_n64Freq=0;

	CLFE_Timer::CLFE_Timer(int nPrecision, bool bPlay)
		: m_n64TimeBegin(0)
		, m_n64TimeEnd(0)
		, m_TimerStatus(tsStop)
	{
		if (m_n64Freq == 0)
		{
			LARGE_INTEGER tmp;
			if (QueryPerformanceFrequency(&tmp) == FALSE)
			{
				pSystem->Log(LOG_ERROR, L"本机无法使用高精度计时器" );
			}
			m_n64Freq = tmp.QuadPart;
		}

		_ASSERT(nPrecision > 0);
		m_nPrecision = nPrecision;

		if (bPlay)
		{
			Play();
		}
	}

	CLFE_Timer::~CLFE_Timer(void)
	{
	}

	DWORD CLFE_Timer::GetTime()
	{
		if (m_TimerStatus != tsRun)
		{
			return DWORD((m_n64TimeEnd - m_n64TimeBegin) * m_nPrecision / m_n64Freq);
		}
		else
		{
			return DWORD((GetCurrentCount() - m_n64TimeBegin) * m_nPrecision / m_n64Freq);
		}
	}

	void CLFE_Timer::Play()
	{
		if (m_TimerStatus == tsStop)
		{
			m_n64TimeBegin = GetCurrentCount();
		}
		m_TimerStatus = tsRun;
	}

	void CLFE_Timer::Stop()
	{
		m_n64TimeEnd = GetCurrentCount();
		m_TimerStatus = tsStop;
	}

	void CLFE_Timer::Pause()
	{
		m_n64TimeEnd = GetCurrentCount();
		m_TimerStatus = tsPause;
	}

	__int64 CLFE_Timer::GetCurrentCount(void)
	{
		LARGE_INTEGER tmp;
		QueryPerformanceCounter(&tmp);
		return tmp.QuadPart;
	}
}