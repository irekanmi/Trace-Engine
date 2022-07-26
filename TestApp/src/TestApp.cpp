
#include <trace.h>
#include <stdio.h>
#include <unordered_map>




trace::trc_app_data trace::CreateApp()
{
	trace::trc_app_data app_data;
	app_data.winprop = trace::WindowDecl();
	app_data.wintype = trace::WindowType::GLFW_WINDOW;
	app_data.windowed = true;

	return app_data;

}