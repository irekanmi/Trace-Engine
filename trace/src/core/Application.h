#pragma once

#include "Core.h"

namespace trace
{

	class TRACE_API Application
	{

	public:
		Application();
		virtual ~Application();

		virtual void Start();
		virtual void Run();
		virtual void End();


	};

	Application* CreateApp();

}

