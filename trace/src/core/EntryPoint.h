#pragma once

#include "Core.h"
#include "Application.h"
#include "StartUp.h"

#ifdef TRC_WINDOWS
extern trace::trc_app_data trace::CreateApp();

int main(int argc, char** argv)
{
	
	if (!trace::INIT())
	{
		TRC_CRITICAL("Engine Startup failed");
		return -1;
	}

	trace::trc_app_data app_data = trace::CreateApp();

	if (!trace::_INIT(app_data))
	{
		TRC_CRITICAL("Engine / Application Startup failed");
		return -1;
	}

	trace::Application::s_instance = new trace::Application(app_data);

	trace::Application::s_instance->Start();
	trace::Application::s_instance->Run();
	trace::Application::s_instance->End();

	trace::_SHUTDOWN(app_data);

	trace::SHUTDOWN();

	return 0;
}

#else

#error Trace currently supports only Windows

#endif
