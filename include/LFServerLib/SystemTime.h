#pragma once

#include "Singleton.h"

#define	ONE_SECOND_IN_USECS			1000000L
#define FILETIME_TO_TIMEVAL_SKEW	INT64_LITERAL(0x19db1ded53e8000)

class CTimeValue
{
public:
	CTimeValue(void);
	explicit CTimeValue(const FILETIME &ft);

	void Set(long sec, long usec);
	void Set(const FILETIME &ft);

	//获取总毫秒数
	UINT64 GetMSec(void);

	void Normalize(void);

	inline CTimeValue &operator = (const CTimeValue &tv)
	{
		this->m_tv.tv_sec = tv.m_tv.tv_sec;
		this->m_tv.tv_usec = tv.m_tv.tv_usec;
		return *this;
	}

	inline CTimeValue &operator -= (const CTimeValue &tv)
	{
		this->m_tv.tv_sec -= tv.m_tv.tv_sec;
		this->m_tv.tv_usec -= tv.m_tv.tv_usec;
		Normalize();
		return *this;
	}
private:
	timeval		m_tv;
};

inline CTimeValue operator - (const CTimeValue &tv1, const CTimeValue &tv2)
{
	CTimeValue delta(tv1);
	delta -= tv2;
	return delta;
}

//////////////////////////////////////////////////////////////////////////

class CSystemTime : public Singleton<CSystemTime>
{
	friend class Singleton<CSystemTime>;
public:
	//返回当前时间
	CTimeValue GetTimeOfDay(void);

	//返回服务器启动的毫秒数
	UINT GetMSTime(void);

	UINT TickTime(void);

	UINT TickPrevTime(void);

	UINT Tick(void);

	inline UINT GetMSTimeDiff(const UINT& oldMSTime, const UINT& newMSTime)
	{
		if (oldMSTime > newMSTime)
		{
			const UINT diff_1 = (UINT(0xFFFFFFFF) - oldMSTime) + newMSTime;
			const UINT diff_2 = oldMSTime - newMSTime;

			return std::min<UINT>(diff_1, diff_2);
		}

		return newMSTime - oldMSTime;
	}

protected:
	CSystemTime(void);

private:
	const CTimeValue	m_SystemTickTime;

	UINT				m_iTime;
	UINT				m_iPrevTime;
};