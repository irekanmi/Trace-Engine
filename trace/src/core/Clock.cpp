#include "pch.h"

#include "Clock.h"

namespace trace {
	Clock::Clock()
		:elapsedTime(0.0f)
	{
	}
	Clock::~Clock()
	{
	}
	void Clock::Begin()
	{
		if (!started)
		{
			start_time = std::chrono::system_clock::now();
			started = true;
		}
	}
	void Clock::Tick( float tick_amount)
	{
		
		elapsedTime += tick_amount;
	}
	float Clock::GetElapsedTime()
	{
		return elapsedTime;
	}
	float Clock::GetInternalElapsedTime()
	{
		if (!started) Begin();
		auto now = std::chrono::system_clock::now();

		//start_time = std::chrono::system_clock::now();
		return std::chrono::duration<float, std::chrono::seconds::period>(now - start_time).count();
	}
}