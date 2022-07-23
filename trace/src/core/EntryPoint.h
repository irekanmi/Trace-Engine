#pragma once

#include "Application.h"


extern trace::Application* trace::CreateApp();

int main()
{
	trace::Logger::init();
	trace::Logger::get_instance()->set_log_level(trace::LogLevel::trace);
	trace::Logger::get_instance()->set_log_name("TRACE");
	trace::Logger::get_instance()->EnableFileLogging();

	trace::Application* app = trace::CreateApp();

	app->Start();
	app->Run();
	app->End();



	delete app;
	app = nullptr;

	return 0;
}
