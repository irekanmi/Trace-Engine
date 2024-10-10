
#include <trace.h>
#include <core/EntryPoint.h> // TODO: Find the reason why we can't add it in the trace header
#include "TraceEditor.h"

using namespace trace;


void Start()
{
	TraceEditor::get_instance()->Init();
}

void Update(float deltaTime)
{
	TraceEditor::get_instance()->Update(deltaTime);
}


void Render(float deltaTime)
{
	TraceEditor::get_instance()->Render(deltaTime);
	
}

void End()
{
	TraceEditor::get_instance()->Shutdown();
	TraceEditor* editor = TraceEditor::get_instance();
	delete editor;
}

trace::trc_app_data trace::CreateApp()
{
	trace::trc_app_data app_data;
	app_data.winprop = trace::WindowDecl("Trace Editor");
	app_data.wintype = trace::WindowType::WIN32_WINDOW;
	app_data.graphics_api = trace::RenderAPI::Vulkan;
	app_data.platform_api = trace::PlatformAPI::WINDOWS;
	app_data.windowed = true;
	app_data.enable_vsync = true;
	app_data.client_start = Start;
	app_data.client_update = Update;
	app_data.client_render = Render;
	app_data.client_end = End;
	app_data.render_composer = TraceEditor::get_instance()->GetRenderComposer(); // TODO
	AppSettings::is_editor = true;


	return app_data;
}
