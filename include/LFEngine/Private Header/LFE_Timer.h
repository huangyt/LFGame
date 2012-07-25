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

		// ���к���
	public:
		//���ü�ʱ������
		void SetPrecision(int nPrecision = 1000)
		{ _ASSERT(nPrecision > 0); m_nPrecision = nPrecision; }

		// ȡ��ʱ������
		int GetPrecision(void) const { return m_nPrecision; }

		// ȡ��ǰ��ʱ������
		DWORD GetTime(void);

		// ��ʼ��ʱ
		void Play(void);

		// ֹͣ��ʱ
		void Stop(void);

		// ��ͣ��ʱ
		void Pause(void);

		// ˽�к���
	private:
		// ȡϵͳ��ʱ���ļ���ֵ
		__int64 GetCurrentCount(void);

		// ��Ա����
	private:
		static __int64 m_n64Freq;		// ��ʱ��Ƶ��
		__int64 m_n64TimeBegin;			// ��ʼʱ��
		__int64 m_n64TimeEnd;			// ֹͣʱ��
		int m_nPrecision;				// ��ʱ������
		TimerStatus m_TimerStatus;		// ��ʱ��״̬
	};
}