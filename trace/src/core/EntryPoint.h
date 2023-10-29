#pragma once

#ifndef TRC_ENTRY
#define TRC_ENTRY

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
	trace::init(app_data);

	trace::Application::s_instance = new trace::Application(app_data); //TODO: Use custom allocator
	if (!trace::_INIT(app_data))
	{
		TRC_CRITICAL("Engine / Application Startup failed");
		return -1;
	}

	trace::Application::s_instance->Start();
	trace::Application::s_instance->Run();
	trace::Application::s_instance->End();

	delete trace::Application::s_instance; //TODO: Use custom allocator

	trace::_SHUTDOWN(app_data);

	trace::SHUTDOWN();

	return 0;
}

#else

#error Trace currently supports only Windows

#endif
#endif
