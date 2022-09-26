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
	trace::Application* app = new trace::Application(app_data);

	app->Start();
	app->Run();
	app->End();

	trace::SHUTDOWN();

	return 0;
}

#else

#error Trace currently supports only Windows

#endif
