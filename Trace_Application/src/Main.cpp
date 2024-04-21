
#include <trace.h>
#include <core/EntryPoint.h> // TODO: Find the reason why we can't add it in the trace header
#include "TraceGame.h"
#include "scene/Entity.h"

using namespace trace;


void Start()
{
	TraceGame::get_instance()->Start();
}

void Update(float deltaTime)
{
	TraceGame::get_instance()->Update(deltaTime);
}


void Render(float deltaTime)
{
	TraceGame::get_instance()->Render(deltaTime);
}

void End()
{
	TraceGame::get_instance()->End();
	TraceGame* game = TraceGame::get_instance();
	delete game;
}

trace::trc_app_data trace::CreateApp()
{
	TraceGame::get_instance()->Init();

	trace::trc_app_data app_data;
	app_data.winprop = trace::WindowDecl(TraceGame::get_instance()->GetName());
	app_data.wintype = trace::WindowType::WIN32_WINDOW;
	app_data.graphics_api = trace::RenderAPI::Vulkan;
	app_data.platform_api = trace::PlatformAPI::WINDOWS;
	app_data.windowed = true;
	app_data.enable_vsync = false;
	app_data.client_start = Start;
	app_data.client_update = Update;
	app_data.client_render = Render;
	app_data.client_end = End;
	app_data.render_composer = nullptr; // TODO


	return app_data;
}
