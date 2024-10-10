#include "pch.h"

#include "Clock.h"

namespace trace {
	Clock::Clock()
	{
	}
	Clock::~Clock()
	{
	}
	void Clock::Begin()
	{
		if (!m_started)
		{
			m_startTime = std::chrono::system_clock::now();
			m_started = true;
			m_elapsedTime = 0.0f;
		}
	}
	void Clock::Tick( float tick_amount)
	{
		
		m_elapsedTime += tick_amount;
	}
	float Clock::GetElapsedTime()
	{
		return m_elapsedTime;
	}
	float Clock::GetInternalElapsedTime()
	{	
		if (!m_started)
		{
			Begin();
		}
		auto now = std::chrono::system_clock::now();

		//start_time = std::chrono::system_clock::now();
		return std::chrono::duration<float, std::chrono::seconds::period>(now - m_startTime).count();
	}
}