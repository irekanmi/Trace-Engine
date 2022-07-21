#pragma once

#include "Application.h"


extern trace::Application* trace::CreateApp();

int main()
{

	trace::Application* app = trace::CreateApp();

	app->Start();
	app->Run();
	app->End();



	delete app;
	app = nullptr;

	return 0;
}
