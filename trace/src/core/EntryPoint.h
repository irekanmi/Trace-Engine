#pragma once

#include "Core.h"
#include "Application.h"
#include "Enums.h"
#include "events\EventsSystem.h"


#ifdef TRC_WINDOWS
extern trace::Application* trace::CreateApp();

int main(int argc, char** argv)
{
	trace::Logger* logger = trace::Logger::get_instance();
	logger->set_log_level(trace::LogLevel::trace);
	logger->set_logger_name("TRACE");
	logger->EnableFileLogging();

	trace::EventsSystem* evs = trace::EventsSystem::get_instance();

	trace::Application* app = trace::CreateApp();

	app->Start();
	app->Run();
	app->End();

	SAFE_DELETE(trace::Logger::get_instance(), 1);
	SAFE_DELETE(evs, e);
	SAFE_DELETE(app, 2)

	return 0;
}

#else

#error Trace currently supports only Windows

#endif
