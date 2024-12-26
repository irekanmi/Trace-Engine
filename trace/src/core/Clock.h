#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include <chrono>

namespace trace {

	class TRACE_API Clock
	{

	public:
		Clock();
		~Clock();

		void Begin();
		void Tick(float tick_amount);
		float GetElapsedTime();
		// Returns elasped time since clock begins "it uses the system clock"
		float GetInternalElapsedTime();

	private:
		std::chrono::high_resolution_clock::time_point m_startTime;
		float m_elapsedTime = 0.0f;
		bool m_started = false;

	protected:

	};

}
