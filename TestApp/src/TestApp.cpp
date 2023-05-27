
#include <trace.h>
#include <stdio.h>
#include <unordered_map>
using namespace trace;

//Temp =================================
#include "glm/gtc/matrix_transform.hpp"
//========================

trace::FileHandle g_fileTest;

class SampleOverLay : public trace::Layer
{

public:
	SampleOverLay()
		:trace::Layer("SampleOverlay")
	{

	}
	~SampleOverLay()
	{

	}

	virtual void OnAttach() override
	{
		TRC_DEBUG("SampleOverlay Attached");
	}
	virtual void OnDetach() override
	{
		TRC_DEBUG("SampleOverlay Detached");
	}
	virtual void Update(float deltaTime) override
	{

	}
	virtual void OnEvent(trace::Event* p_event) override
	{
		switch (p_event->m_type)
		{
		case trace::EventType::TRC_KEY_RELEASED:
		{
			trace::KeyReleased* press = reinterpret_cast<trace::KeyReleased*>(p_event);
			if (press->m_keycode == trace::Keys::KEY_ESCAPE)
			{
				press->m_handled = true;
				trace::Application::get_instance()->PopOverLay(this);
			}
			break;
		}
		}
	}

private:
protected:

};

class SampleLayer : public trace::Layer
{

public:
	SampleLayer()
		:trace::Layer("SampleLayer")
	{
	}

	~SampleLayer()
	{

	}

	virtual void OnAttach() override
	{
		TRC_INFO("Sample Layer Attached");

		_boxStack = ResourceSystem::get_instance()->LoadMesh("box_stack.obj");
		_falcon = ResourceSystem::get_instance()->LoadMesh("falcon.obj");

	}

	virtual void OnEvent(trace::Event* p_event) override
	{
		switch (p_event->m_type)
		{
		case trace::EventType::TRC_KEY_PRESSED:
		{
			trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);
			//printf("%c", press->m_keycode);
			break;
		}
		case trace::EventType::TRC_KEY_RELEASED:
		{
			trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);
			break;
		}
		}
	}

	virtual void OnDetach() override
	{
		TRC_WARN("Sample Layer detached");
	}

	virtual void Update(float deltaTime) override
	{

		if (trace::InputSystem::get_instance()->GetKeyState(trace::Keys::KEY_T) == trace::KeyState::KEY_RELEASE)
		{
			TRC_INFO(" ------____----TRACE------______----");
		}

		Renderer* renderer = Renderer::get_instance();
		CommandList cmd_list = renderer->BeginCommandList();
		renderer->DrawMesh(cmd_list, _boxStack, M_boxStack.GetLocalMatrix());
		renderer->DrawMesh(cmd_list, _falcon, M_falcon.GetLocalMatrix());
		renderer->SubmitCommandList(cmd_list);

	}
private:
	//Temp ==================
	Ref<Mesh> _squareModel;
	Ref<Mesh> _sponzaScene;
	Ref<Mesh> _falcon;
	Ref<Mesh> _boxStack;

	Transform M_squareModel;
	Transform M_sponzaScene;
	Transform M_falcon;
	Transform M_boxStack;
	SkyBox sky_box;
	//=======================
protected:


};



void Start()
{
	TRC_INFO("Client Start");
	trace::Application::get_instance()->PushLayer(new SampleLayer);
	trace::Application::get_instance()->PushOverLay(new SampleOverLay());

	
	
}

void Update(float deltaTime)
{
	
}

void End()
{
	TRC_WARN("Client End");
}

trace::trc_app_data trace::CreateApp()
{
	trace::trc_app_data app_data;
	app_data.winprop = trace::WindowDecl();
	app_data.wintype = trace::WindowType::GLFW_WINDOW;
	app_data.graphics_api = trace::RenderAPI::Vulkan;
	app_data.platform_api = trace::PlatformAPI::WINDOWS;
	app_data.windowed = true;
	app_data.enable_vsync = false;
	app_data.client_start = Start;
	app_data.client_update = Update;
	app_data.client_end = End;


	return app_data;

}



