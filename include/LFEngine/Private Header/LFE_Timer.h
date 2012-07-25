#pragma once

#include "LFEngine.h"

namespace LF
{
	class CLFE_Timer
	{
	private:
		enum TimerStatus { tsRun, tsStop, tsPause };

	public:
		CLFE_Timer(int nPrecision = 1000, bool bPlay = true);
		~CLFE_Timer(void);

		// 公有函数
	public:
		//设置计时器精度
		void SetPrecision(int nPrecision = 1000)
		{ _ASSERT(nPrecision > 0); m_nPrecision = nPrecision; }

		// 取计时器精度
		int GetPrecision(void) const { return m_nPrecision; }

		// 取当前计时器读数
		DWORD GetTime(void);

		// 开始计时
		void Play(void);

		// 停止计时
		void Stop(void);

		// 暂停计时
		void Pause(void);

		// 私有函数
	private:
		// 取系统计时器的计数值
		__int64 GetCurrentCount(void);

		// 成员变量
	private:
		static __int64 m_n64Freq;		// 计时器频率
		__int64 m_n64TimeBegin;			// 开始时刻
		__int64 m_n64TimeEnd;			// 停止时刻
		int m_nPrecision;				// 计时器精度
		TimerStatus m_TimerStatus;		// 计时器状态
	};
}