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
		void Tick();
		float GetElapsedTime();

	private:
		std::chrono::time_point<std::chrono::steady_clock> start_time;
		float elapsedTime;

	protected:

	};

}
