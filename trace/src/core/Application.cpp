#include "pch.h"
#include "Application.h"
#include "io/Logging.h"
#include "Enums.h"
#include "events/Events.h"
#include "events/EventsSystem.h"
#include "platform\GLFW\GLFWwindow.h"
#include "input\Input.h"
#include "render/Renderer.h"
#include "core/platform/Windows/Win32Window.h"
#include "glm/gtc/matrix_transform.hpp"
#include "memory/memory.h"

//Temp==================
#include "render/Graphics.h"
#include "glm/glm.hpp"
#include "render/Renderutils.h"
#include "render/ShaderParser.h"
//======================


void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new char[size];
}

// implement alignment
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new char[size];
}



namespace trace
{
	extern eastl::vector<trace::Object*> g_SystemPtrs;
	
	
	Application* Application::s_instance = nullptr;

	Application* Application::get_instance()
	{
		return s_instance;
	}


	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack->PushLayer(layer);
	}
	void Application::PushOverLay(Layer* layer)
	{
		m_LayerStack->PushOverLay(layer);
	}
	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack->PopLayer(layer);
	}
	void Application::PopOverLay(Layer* layer)
	{
		m_LayerStack->PopOverLay(layer);
	}

	eastl::vector<Object*> Application::GetEngineSystemsID()
	{
		return g_SystemPtrs;
	}

	Application::Application(trc_app_data appData)
	{


		m_LayerStack = new LayerStack();
		switch (appData.wintype)
		{
		case WindowType::GLFW_WINDOW:
		{
			this->m_Window = new GLFW_Window(appData.winprop);
			break;
		}
		case WindowType::WIN32_WINDOW:
		{
			this->m_Window = new Win32Window(appData.winprop);
			break;
		}
		}

		m_client_start = appData.client_start;
		m_client_update = appData.client_update;
		m_client_end = appData.client_end;

		m_vsync = appData.enable_vsync;

		for (unsigned int i = 1; i < EventType::MAX_EVENTS; i++)
		{
			trace::EventsSystem::get_instance()->AddEventListener((EventType)i, BIND_EVENT_FN(Application::OnEvent));
		}

	}

	Application::~Application()
	{



	}

	void Application::Start()
	{
		


		trace::ApplicationStart app_start;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_START, &app_start);

		//Temp=======================
		






		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_format = Format::R8G8B8A8_UNORM;
		texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
		texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
		texture_desc.m_usage = UsageFlag::DEFAULT;


		Material _mat;
		_mat.m_albedoMap = ResourceSystem::get_instance()->LoadTexture("cobblestone.png");
		//_mat.m_albedoMap = ResourceSystem::s_instance->GetDefaultTexture("albedo_map");
		_mat.m_diffuseColor = glm::vec4(0.3f, 0.5f, 0.45f, 1.0f);
		_mat.m_shininess = 64.0f;
		//_mat.m_specularMap = ResourceSystem::s_instance->GetDefaultTexture("specular_map");
		_mat.m_specularMap = ResourceSystem::get_instance()->LoadTexture("cobblestone_SPEC.png");
		_mat.m_normalMap = ResourceSystem::get_instance()->LoadTexture("cobblestone_NRM.png", texture_desc);
		//_mat.m_normalMap = ResourceSystem::s_instance->GetDefaultTexture("normal_map");

		//_squareModel = ResourceSystem::get_instance()->GetDefaultMesh("Cube");
		_squareModel = ResourceSystem::get_instance()->LoadMesh("box_stack.obj");

		
		std::vector<std::string> cube_maps = {
			"sky_right.jpg",
			"sky_left.jpg",
			"sky_top.jpg",
			"sky_bottom.jpg",
			"sky_front.jpg",
			"sky_back.jpg"
		};

		TextureDesc cubeMapDesc;
		cubeMapDesc.m_image_type = ImageType::CUBE_MAP;
		cubeMapDesc.m_addressModeU = cubeMapDesc.m_addressModeV = cubeMapDesc.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		cubeMapDesc.m_numLayers = 6;
		cubeMapDesc.m_format = Format::R8G8B8A8_SRBG;
		cubeMapDesc.m_minFilterMode = cubeMapDesc.m_magFilterMode = FilterMode::LINEAR;
		cubeMapDesc.m_usage = UsageFlag::DEFAULT;
		cubeMapDesc.m_flag = BindFlag::SHADER_RESOURCE_BIT;

		Texture_Ref temp = ResourceSystem::get_instance()->LoadTexture(cube_maps, cubeMapDesc, "cube_map");

		sky_box = SkyBox(temp);
		



		std::string vert_src;
		std::string frag_src;

		vert_src = ShaderParser::load_shader_file("../assets/shaders/trace_core.shader.vert.glsl");
		frag_src = ShaderParser::load_shader_file("../assets/shaders/reflect.frag.glsl");

		std::cout << vert_src;
		std::cout << frag_src;

		GShader* VertShader = GShader::Create_(vert_src, ShaderStage::VERTEX_SHADER);
		GShader* FragShader = GShader::Create_(frag_src, ShaderStage::PIXEL_SHADER);

		ShaderResourceBinding projection;
		projection.shader_stage = ShaderStage::VERTEX_SHADER;
		projection.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		projection.resource_size = sizeof(glm::mat4);
		projection.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		projection.resource_name = "projection";
		projection.count = 1;
		projection.index = 0;
		projection.slot = 0;
		projection.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		ShaderResourceBinding view;
		view.shader_stage = ShaderStage::VERTEX_SHADER;
		view.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		view.resource_size = sizeof(glm::mat4);
		view.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		view.resource_name = "view";
		view.count = 1;
		view.index = 0;
		view.slot = 0;
		view.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		ShaderResourceBinding view_position;
		view_position.shader_stage = ShaderStage::VERTEX_SHADER;
		view_position.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		view_position.resource_size = sizeof(glm::vec3);
		view_position.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		view_position.resource_name = "view_position";
		view_position.count = 1;
		view_position.index = 0;
		view_position.slot = 0;
		view_position.resource_data_type = ShaderData::CUSTOM_DATA_VEC3;

		ShaderResourceBinding _test;
		_test.shader_stage = ShaderStage::VERTEX_SHADER;
		_test.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		_test.resource_size = sizeof(glm::vec2);
		_test.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		_test.resource_name = "_test";
		_test.count = 1;
		_test.index = 0;
		_test.slot = 0;
		_test.resource_data_type = ShaderData::CUSTOM_DATA_VEC2;

		ShaderResourceBinding normal_map;
		normal_map.shader_stage = ShaderStage::PIXEL_SHADER;
		normal_map.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		normal_map.resource_size = 0;
		normal_map.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		normal_map.resource_name = "normal_map";
		normal_map.count = 1;
		normal_map.index = 0;
		normal_map.slot = 1;
		normal_map.resource_data_type = ShaderData::MATERIAL_NORMAL;


		ShaderResourceBinding rest;
		rest.shader_stage = ShaderStage::PIXEL_SHADER;
		rest.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		rest.resource_size = sizeof(glm::ivec4);
		rest.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		rest.resource_name = "rest";
		rest.count = 1;
		rest.index = 0;
		rest.slot = 2;
		rest.resource_data_type = ShaderData::CUSTOM_DATA_VEC4;

		ShaderResourceBinding cube_data;
		cube_data.shader_stage = ShaderStage::PIXEL_SHADER;
		cube_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		cube_data.resource_size = 0;
		cube_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		cube_data.resource_name = "CubeMap";
		cube_data.count = 1;
		cube_data.index = 0;
		cube_data.slot = 1;
		cube_data.resource_data_type = ShaderData::CUSTOM_DATA_TEXTURE;
		cube_data.data = temp.get();

		ShaderResourceBinding model;
		model.shader_stage = ShaderStage::VERTEX_SHADER;
		model.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
		model.resource_size = sizeof(glm::mat4);
		model.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		model.resource_name = "model";
		model.count = 1;
		model.index = 0;
		model.slot = 3;
		model.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		std::vector<ShaderResourceBinding> scene = {
			projection,
			view,
			view_position,
			_test,
			normal_map,
			cube_data,
			model,
			rest
		};

		PipelineStateDesc _ds;
		_ds.resource_bindings_count = 8;
		_ds.resource_bindings = scene;
		_ds.vertex_shader = VertShader;
		_ds.pixel_shader = FragShader;

		AutoFillPipelineDesc(
			_ds
		);

		if (!ResourceSystem::get_instance()->CreatePipeline(_ds, "reflect_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create reflect pipeline");
		}

		Ref<GPipeline> reflect_p = ResourceSystem::get_instance()->GetPipeline("reflect_pipeline");
		delete VertShader;
		delete FragShader;


		ResourceSystem::get_instance()->CreateMaterial(
			"Material",
			_mat,
			//ResourceSystem::get_instance()->GetDefaultPipeline("standard")
			reflect_p
		);

		//for (auto& i : _squareModel->GetModels())
		//{
		//	i->m_matInstance = ResourceSystem::get_instance()->GetMaterial("Material");
		//}


		//=============================

		//----------CLIENT--------------//
		m_client_start();
		//______________________________//
	}
	
	void Application::Run()
	{
		TRC_WARN("Trace Engine {}", "in progress");

		InputSystem* input = InputSystem::get_instance();
		Renderer* renderer = Renderer::get_instance();
		
		//Temp------------------------
		renderer->Start();
		//----------------------------

		m_clock.Begin();

		while (m_isRunning)
		{
			float _time = m_clock.GetElapsedTime();
			float deltaTime = _time - m_lastTime;
			m_lastTime = _time;

			m_Window->Update(0.0f);
			for (int i = m_LayerStack->Size() - 1; i >= 0; i--)
			{
				Layer* layer = m_LayerStack->m_Layers[i];
				layer->Update(deltaTime);
			}

			//------CLIENT-------//

			m_client_update(deltaTime);

			//___________________//	


			CommandList cmd_list = renderer->BeginCommandList();
			renderer->DrawMesh(cmd_list, _squareModel);
			renderer->SubmitCommandList(cmd_list);

			cmd_list = renderer->BeginCommandList();
			renderer->DrawSky(cmd_list, &sky_box);
			renderer->SubmitCommandList(cmd_list);


			renderer->Render(deltaTime);

			input->Update(deltaTime);
			float end_time = m_clock.GetElapsedTime();
			float total_frame_time = end_time - _time;
			float frame_per_sec = 1.0f / 55;

			//TODO fix, Application is lock to 60 FPS and not found the reason
			if (m_vsync)
			{
				if (total_frame_time < frame_per_sec && total_frame_time > 0.0f)
				{
					float value = frame_per_sec - total_frame_time;
					Platform::Sleep((value * 1000.0f));
				}
			}
		}
	}

	void Application::End()
	{

		//------CLIENT-------//

		m_client_end();

		//___________________//
		trace::ApplicationEnd app_end;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_END, &app_end);
		
		Renderer::get_instance()->End();

		SAFE_DELETE(m_Window, Window);
		m_LayerStack->Shutdown();
		SAFE_DELETE(m_LayerStack, LayerStack);
	}

	void Application::OnEvent(Event* p_event)
	{


		// TODO: Maybe all event should be sent to Layers by the application or layers should register themself to the Event System
		int i = m_LayerStack->Size() - 1;
		for (; i >= 0; --i)
		{
			Layer* layer = m_LayerStack->m_Layers[i];
			if(!p_event->m_handled)
			{
				layer->OnEvent(p_event);
			}
		}

		if (!p_event->m_handled) {
			//TRC_TRACE("Event: {}", p_event->GetName());
			switch (p_event->m_type)
			{
			case trace::EventType::TRC_APP_START:
			{
				trace::ApplicationStart* app_start = reinterpret_cast<trace::ApplicationStart*>(p_event);
				TRC_DEBUG(app_start->Log());
				app_start->m_handled = true;
				break;
			}
			case trace::EventType::TRC_APP_END:
			{
				trace::ApplicationEnd* app_end = reinterpret_cast<trace::ApplicationEnd*>(p_event);
				//TRC_WARN(app_end->Log());
				app_end->m_handled = true;
				break;
			}
			case trace::EventType::TRC_WND_CLOSE:
			{
				trace::WindowClose* wnd_close = reinterpret_cast<trace::WindowClose*>(p_event);
				m_isRunning = false;
				//wnd_close->m_handled = true;
				break;
			}
			case trace::EventType::TRC_KEY_PRESSED:
			{
				KeyPressed* press = reinterpret_cast<KeyPressed*>(p_event);
				break;
			}
			case trace::EventType::TRC_KEY_RELEASED:
			{
				KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);
				if (release->m_keycode == Keys::KEY_ESCAPE)
				{
					m_isRunning = false;
				}
				break;
			}

			case EventType::TRC_WND_RESIZE:
			{
				WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
				if (wnd->m_width == 0 || wnd->m_height == 0)
					break;

				break;
			}

			case EventType::TRC_BUTTON_PRESSED:
			{
				MousePressed* mouse = reinterpret_cast<MousePressed*>(p_event);
				TRC_CRITICAL("Button: {}", mouse->m_button);
				break;
			}

			case EventType::TRC_BUTTON_RELEASED:
			{
				MouseReleased* mouse = reinterpret_cast<MouseReleased*>(p_event);
				TRC_TRACE("Button: {}", mouse->m_button);
				break;
			}

			case EventType::TRC_MOUSE_MOVE:
			{
				MouseMove* mouse = reinterpret_cast<MouseMove*>(p_event);
				//TRC_WARN("X: {}", mouse->m_x);
				//TRC_WARN("Y: {}", mouse->m_y);
				break;
			}
			}
		}
	}

}