
#include <trace.h>
#include "imgui.h"

using namespace trace;




void Start()
{

}

void Update(float deltaTime)
{

}


static bool show_demo = true;
void Render(float deltaTime)
{
	
	if(show_demo) ImGui::ShowDemoWindow(&show_demo);
	if (ImGui::Begin("Sample Tab"))
	{
		ImGui::Text("Trace Engine");
		if (ImGui::Button("Show Demo Window"))
		{
			show_demo = true;
		}
		ImGui::End();
	}
}

void End()
{

}

trace::trc_app_data trace::CreateApp()
{
	trace::trc_app_data app_data;
	app_data.winprop = trace::WindowDecl("Trace Editor");
	app_data.wintype = trace::WindowType::WIN32_WINDOW;
	app_data.graphics_api = trace::RenderAPI::Vulkan;
	app_data.platform_api = trace::PlatformAPI::WINDOWS;
	app_data.windowed = true;
	app_data.enable_vsync = false;
	app_data.client_start = Start;
	app_data.client_update = Update;
	app_data.client_render = Render;
	app_data.client_end = End;


	return app_data;
}
