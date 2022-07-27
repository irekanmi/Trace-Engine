#pragma once

#include "Core.h"
#include "Application.h"


#ifdef TRC_WINDOWS
extern trace::Application* trace::CreateApp();

int main(int argc, char** argv)
{
	trace::Logger::init();
	trace::Logger* logger = trace::Logger::get_instance();
	logger->set_log_level(trace::LogLevel::trace);
	logger->set_log_name("TRACE");
	logger->EnableFileLogging();

	trace::Application* app = trace::CreateApp();

	app->Start();
	app->Run();
	app->End();

	SAFE_DELETE(logger)
	SAFE_DELETE(app)

	return 0;
}

#else

#error Trace currently supports only windows

#endif
