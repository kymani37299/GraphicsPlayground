#pragma once

#include "Common.h"

#include <chrono>

namespace GP
{
	class Timer
	{
	public:
		void Start()
		{
			m_Running = true;
			m_BeginTime = std::chrono::high_resolution_clock::now();
		}

		void Stop()
		{
			m_Running = false;
			m_TimeMS = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - m_BeginTime).count();
		}

		inline float GetTimeMS() const
		{
			ASSERT(!m_Running, "[Timer] Trying to get time on running timer!");
			return m_TimeMS;
		}

	private:
		std::chrono::time_point<std::chrono::steady_clock> m_BeginTime;
		float m_TimeMS = 0.0f;
		bool m_Running = false;
	};
}