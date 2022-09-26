#pragma once

#include "Core.h"
#include "events\Events.h"
#include "Window.h"

namespace trace
{

	struct trc_app_data
	{
		trc_app_data(){}
		WindowDecl winprop;
		WindowType wintype;
	};

	class TRACE_API Application
	{

	public:
		Application(trc_app_data appData);
		virtual ~Application();

		virtual void Start();
		virtual void Run();
		virtual void End();

		virtual void OnEvent(Event* p_event);

	private:
	protected:
		Window* m_Window;
		bool m_isRunning = true;

	};

	trc_app_data CreateApp();

}

