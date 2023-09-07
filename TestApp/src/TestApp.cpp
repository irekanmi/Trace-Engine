
#include <trace.h>
#include <stdio.h>
#include <unordered_map>
using namespace trace;

//Temp =================================
#include "glm/gtc/matrix_transform.hpp"
//========================


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
		//_falcon = ResourceSystem::get_instance()->LoadMesh("falcon.obj");
		//_sponzaScene = ResourceSystem::get_instance()->LoadMesh("sponza.obj");
		_sphereModel = ResourceSystem::get_instance()->GetDefaultMesh("Sphere");

		TextureDesc sky = {};
		sky.m_addressModeU = sky.m_addressModeV = sky.m_addressModeW = AddressMode::REPEAT;
		sky.m_format = Format::R8G8B8A8_SRBG;
		sky.m_image_type = ImageType::CUBE_MAP;
		sky.m_usage = UsageFlag::DEFAULT;
		sky.m_minFilterMode = sky.m_magFilterMode = FilterMode::LINEAR;
		sky.m_flag = BindFlag::SHADER_RESOURCE_BIT;

		std::vector<std::string> sky_images = {
			"sky_right.jpg",
			"sky_left.jpg",
			"sky_top.jpg",
			"sky_bottom.jpg",
			"sky_front.jpg",
			"sky_back.jpg"
		};

		Ref<GTexture> sky_texture;
		sky_texture = ResourceSystem::get_instance()->LoadTexture(sky_images, sky, "bright_sky");

		sky_box = SkyBox(sky_texture);

		M_sponzaScene.SetScale(glm::vec3(0.15f));
		M_falcon.Translate(glm::vec3(3.0f, 7.5f, 0.0f));
		M_boxStack.Translate(glm::vec3(3.0f, 17.5f, 0.0f));
		M_Test.Translate(glm::vec3(13.0f, 57.5f, 0.0f));
		M_falcon.SetScale(glm::vec3(3.0f));
		M_boxStack.SetScale(glm::vec3(5.5f));
		M_Test.SetScale(glm::vec3(1.0f));
	}

	virtual void OnEvent(trace::Event* p_event) override
	{
		switch (p_event->m_type)
		{
		case trace::EventType::TRC_KEY_PRESSED:
		{
			trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);
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

		M_boxStack.Rotate(12.0f * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

		Renderer* renderer = Renderer::get_instance();
		CommandList cmd_list = renderer->BeginCommandList();
		//renderer->DrawMesh(cmd_list, _falcon, M_falcon.GetLocalMatrix());
		renderer->DrawMesh(cmd_list, _boxStack, M_boxStack.GetLocalMatrix());
		renderer->DrawMesh(cmd_list, _sphereModel, M_Test.GetLocalMatrix());
		//renderer->DrawMesh(cmd_list, _sponzaScene, M_sponzaScene.GetLocalMatrix());
		renderer->DrawSky(cmd_list, &sky_box);
		renderer->SubmitCommandList(cmd_list);

	}
private:
	//Temp ==================
	Ref<Mesh> _squareModel;
	Ref<Mesh> _sphereModel;
	Ref<Mesh> _sponzaScene;
	Ref<Mesh> _falcon;
	Ref<Mesh> _boxStack;

	Transform M_squareModel;
	Transform M_sponzaScene;
	Transform M_falcon;
	Transform M_boxStack;
	Transform M_Test;
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
	app_data.wintype = trace::WindowType::WIN32_WINDOW;
	app_data.graphics_api = trace::RenderAPI::Vulkan;
	app_data.platform_api = trace::PlatformAPI::WINDOWS;
	app_data.windowed = true;
	app_data.enable_vsync = false;
	app_data.client_start = Start;
	app_data.client_update = Update;
	app_data.client_end = End;


	return app_data;

}



