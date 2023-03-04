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
		start_time = std::chrono::steady_clock::now();
	}
	void Clock::Tick()
	{
		auto now = std::chrono::steady_clock::now();

		elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(now - start_time).count();
		//start_time = std::chrono::steady_clock::now();
	}
	float Clock::GetElapsedTime()
	{
		auto now = std::chrono::steady_clock::now();
		return std::chrono::duration<float, std::chrono::seconds::period>(now - start_time).count();
	}
}