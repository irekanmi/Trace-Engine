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
	void Clock::Tick()
	{
		auto now = std::chrono::system_clock::now();

		elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(now - start_time).count();
		//start_time = std::chrono::system_clock::now();
	}
	float Clock::GetElapsedTime()
	{
		if (!started) Begin();
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration<float>(now - start_time).count();
	}
}