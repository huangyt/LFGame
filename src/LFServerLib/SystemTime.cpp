#include "GlobalDef.h"
#include "SystemTime.h"

CTimeValue::CTimeValue()
{
	Set(0, 0);
}

CTimeValue::CTimeValue(const FILETIME &ft)
{
	Set(ft);
}

void CTimeValue::Set(long sec, long usec)
{
	m_tv.tv_sec = sec;
	m_tv.tv_usec = usec;

	Normalize();
}

UINT64 CTimeValue::GetMSec()
{
	UINT64 ms = m_tv.tv_sec;
	ms *= 1000;
	ms += m_tv.tv_usec / 1000;
	return ms;
}

void CTimeValue::Set(const FILETIME &ft)
{
	ULARGE_INTEGER _100ns;
	_100ns.LowPart = ft.dwLowDateTime;
	_100ns.HighPart = ft.dwHighDateTime;
	_100ns.QuadPart -= FILETIME_TO_TIMEVAL_SKEW;

	m_tv.tv_sec = (long) (_100ns.QuadPart / (10000 * 1000));
	m_tv.tv_usec = (long) ((_100ns.QuadPart % (10000 * 1000)) / 10);

	Normalize();
}

void CTimeValue::Normalize()
{
	if(m_tv.tv_usec >= ONE_SECOND_IN_USECS)
	{
		do
		{
			++m_tv.tv_sec;
			m_tv.tv_usec -= ONE_SECOND_IN_USECS;
		}
		while (m_tv.tv_usec >= ONE_SECOND_IN_USECS);
	}
	else if(m_tv.tv_usec <= -ONE_SECOND_IN_USECS)
	{
		do
		{
			--m_tv.tv_sec;
			m_tv.tv_usec += ONE_SECOND_IN_USECS;
		}
		while (m_tv.tv_usec <= -ONE_SECOND_IN_USECS);
	}

	if (m_tv.tv_sec >= 1 && m_tv.tv_usec < 0)
	{
		--m_tv.tv_sec;
		m_tv.tv_usec += ONE_SECOND_IN_USECS;
	}
	else if (m_tv.tv_sec < 0 && m_tv.tv_usec > 0)
	{
		++m_tv.tv_sec;
		m_tv.tv_usec -= ONE_SECOND_IN_USECS;
	}
}

//////////////////////////////////////////////////////////////////////////
CSystemTime::CSystemTime()
	:m_SystemTickTime(GetTimeOfDay())
	,m_iTime(0)
	,m_iPrevTime(0)
{
}

CTimeValue CSystemTime::GetTimeOfDay()
{
	FILETIME tfile;
	::GetSystemTimeAsFileTime (&tfile);
	return CTimeValue(tfile);
}

UINT CSystemTime::TickTime()
{
	return m_iTime;
}

UINT CSystemTime::TickPrevTime()
{
	return m_iPrevTime;
}

UINT CSystemTime::Tick()
{
	m_iPrevTime = m_iTime;

	m_iTime = GetMSTime();

	return GetMSTimeDiff(m_iPrevTime, m_iTime);
}

UINT CSystemTime::GetMSTime()
{
	const CTimeValue currTime = GetTimeOfDay();

	UINT64 diff = (currTime - m_SystemTickTime).GetMSec();

	return (UINT)(diff % UINT64_LITERAL(0x00000000FFFFFFFF));
}