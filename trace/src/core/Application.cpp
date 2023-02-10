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
#include "render/Graphics.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"


void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	//TRC_INFO("size: %d, name: %s, file: %s, line: %d");
	//printf("size: %d, name: %s, file: %s, line: %d \n", size, pName, file, line);
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
	WindowType Application::win_type = WindowType::DEFAULT;

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

		m_Window->SetVsync(appData.enable_vsync);

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

		//----------CLIENT--------------//
		m_client_start();
		//______________________________//


		trace::FileHandle vert_shad;
		trace::FileHandle frag_shad;

		std::string vert_src;
		std::string frag_src;

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.vert.glsl", trace::FileMode::READ, vert_shad))
		{
			TRC_ERROR("Failed to open file");
		}

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.frag.glsl", trace::FileMode::READ, frag_shad))
		{
			TRC_ERROR("Failed to open file");
		}

		trace::FileSystem::read_all_lines(vert_shad, vert_src);
		trace::FileSystem::read_all_lines(frag_shad, frag_src);

		std::cout << vert_src;
		std::cout << frag_src;

		trace::FileSystem::close_file(vert_shad);
		trace::FileSystem::close_file(frag_shad);

		VertShader = GShader::Create_(vert_src, ShaderStage::VERTEX_SHADER);
		FragShader = GShader::Create_(frag_src, ShaderStage::PIXEL_SHADER);

		ColorBlendState blend_state;
		blend_state.alpha_to_blend_coverage = false;

		DepthStencilState depth_stenc_state;
		depth_stenc_state.depth_test_enable = true;
		depth_stenc_state.maxDepth = 1.0f;
		depth_stenc_state.minDepth = 0.0f;
		depth_stenc_state.stencil_test_enable = false;

		InputLayout _layout = Vertex::get_input_layout();

		RaterizerState raterizer_state;
		raterizer_state.cull_mode = CullMode::BACK;
		raterizer_state.fill_mode = FillMode::SOLID;

		Viewport vp = {};
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = GetWindow()->GetWidth();
		vp.height = GetWindow()->GetHeight();
		vp.x = 0;
		vp.y = 0;

		PipelineStateDesc desc;
		desc.blend_state = blend_state;
		desc.depth_sten_state = depth_stenc_state;
		desc.input_layout = _layout;
		desc.pixel_shader = FragShader;
		desc.rateriser_state = raterizer_state;
		desc.topology = PrimitiveTopology::TRIANGLE_LIST;
		desc.vertex_shader = VertShader;
		desc.view_port = vp;
		
		_pipeline = GPipeline::Create_(desc);

		const float scale = 10;


		m_vertices = {
			{ {-0.5f, -0.5f, 0.0f}, { 0.85f, 0.55f, 0.75f }},
			{ {0.5f, -0.5f, 0.0f}, {0.55f, .75f, .85f} },
			{ {0.5f, 0.5f, 0.0f}, {.75f, .25f, .55f} },
			{ { -0.5f, 0.5f, 0.0f }, { 0.95f, 0.25f, .00f }}
		};

		for (auto& i : m_vertices)
		{
			i.pos *= scale;
		}



		m_indices = {
			0, 1, 2,
			0, 2, 3
		};

		BufferInfo vertex_buffer_info;
		vertex_buffer_info.m_size = m_vertices.size() * sizeof(Vertex);
		vertex_buffer_info.m_stide = sizeof(Vertex);
		vertex_buffer_info.m_usage = BufferUsage::VERTEX_BUFFER;
		vertex_buffer_info.m_data = m_vertices.data();

		BufferInfo index_buffer_info;
		index_buffer_info.m_size = m_indices.size() * sizeof(uint32_t);
		index_buffer_info.m_stide = sizeof(uint32_t);
		index_buffer_info.m_usage = BufferUsage::INDEX_BUFFER;
		index_buffer_info.m_data = m_indices.data();

		VertexBuffer = GBuffer::Create_(vertex_buffer_info);
		IndexBuffer = GBuffer::Create_(index_buffer_info);

#if 0
		std::string vert_src = R"(
			#version 330 core

			layout(location = 0)in vec3 a_pos;
			layout(location = 1)in vec3 a_color;

			out vec3 _color;

			void main()
			{
				_color = a_color;
				gl_Position = vec4(a_pos, 1.0f);
			}
		)";

		std::string frag_src = R"(
			#version 330 core

			layout(location = 0)out vec4 FragColor;

			in vec3 _color;

			void main()
			{
				FragColor = vec4(_color , 1.0f);
			}
		)";

		Shader = glCreateProgram();
		uint32_t vert_shader = glCreateShader(GL_VERTEX_SHADER);
		uint32_t frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
		const char* vs = vert_src.c_str();
		const char* fs = frag_src.c_str();

		glShaderSource(vert_shader, 1, &vs, nullptr);
		glShaderSource(frag_shader, 1, &fs, nullptr);

		int success;
		int lenght = 1024;
		char buffer[1024]{};
		glCompileShader(vert_shader);
		glCompileShader(frag_shader);

		glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vert_shader, lenght, nullptr, buffer);
			TRC_ERROR("Vertex Shader Error: \n %s", buffer);
		}

		glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(frag_shader, lenght, nullptr, buffer);
			TRC_ERROR("Fragment Shader Error: \n %s", buffer);
		}

		glAttachShader(Shader, vert_shader);
		glAttachShader(Shader, frag_shader);
		glLinkProgram(Shader);

		glGetProgramiv(Shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(Shader, lenght, &lenght, buffer);
			TRC_ERROR("Link Error: \n %s", buffer);
		}
		BufferInfo vertex_buffer_info;
		vertex_buffer_info.m_size = m_vertices.size() * sizeof(Vertex);
		vertex_buffer_info.m_stide = sizeof(Vertex);
		vertex_buffer_info.m_usage = BufferUsage::VERTEX_BUFFER;
		vertex_buffer_info.m_data = m_vertices.data();

		BufferInfo index_buffer_info;
		index_buffer_info.m_size = m_indices.size() * sizeof(uint32_t);
		index_buffer_info.m_stide = sizeof(uint32_t);
		index_buffer_info.m_usage = BufferUsage::INDEX_BUFFER;
		index_buffer_info.m_data = m_indices.data();

		VertexBuffer = GBuffer::Create_(vertex_buffer_info);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));


		IndexBuffer = GBuffer::Create_(index_buffer_info);
#endif


	}
	
	void Application::Run()
	{
		printf("Application Running\n");


		TRC_WARN("Trace Engine %s", "in progress");


		

		InputSystem* input = InputSystem::get_instance();
		Renderer* renderer = Renderer::get_instance();
		
		//Temp------------------------
		renderer->UsePipeline(_pipeline);
		//----------------------------

		while (m_isRunning)
		{
			m_Window->Update(0.0f);
			

			for (int i = m_LayerStack->Size() - 1; i >= 0; i--)
			{
				Layer* layer = m_LayerStack->m_Layers[i];
				layer->Update(0.0f);
			}

			//------CLIENT-------//

			m_client_update(0.0f);

			//___________________//
#if 0
			renderer->BeginScene();
			
			glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(Shader);

			renderer->Draw(IndexBuffer);

			renderer->EndScene();

#endif		

			if (renderer->BeginFrame())
			{
				SceneGlobals scene_data = {};
				scene_data.view = glm::identity<glm::mat4>();
				static float z = -5.0f;
				z -= 0.01f;

				scene_data.view = glm::translate(scene_data.view, glm::vec3(.0f, .0f, z));
				scene_data.projection = glm::identity<glm::mat4>();
				scene_data.projection = glm::perspective(45.0f, 800.0f / 600.0f, 0.1f, 500.0f);

				renderer->UpdateSceneGlobalData(scene_data);

				renderer->BindPipeline(_pipeline);
				renderer->BindVertexBuffer(VertexBuffer);
				renderer->BindIndexBuffer(IndexBuffer);

				renderer->DrawIndexed(0, 6);

				renderer->EndFrame();
			}

			renderer->Update(0.0f);
			input->Update(0.0f);
		}
	}

	void Application::End()
	{


		//------CLIENT-------//

		m_client_end();

		//___________________//
		trace::ApplicationEnd app_end;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_END, &app_end);


		// Temp-----------------------------

		delete _pipeline;

		delete VertexBuffer;
		delete IndexBuffer;

		delete VertShader;
		delete FragShader;

		//----------------------------------

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
			//TRC_TRACE("Event: %s", p_event->GetName());
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
				wnd_close->m_handled = true;
				break;
			}
			case trace::EventType::TRC_KEY_PRESSED:
			{
				KeyPressed* press = reinterpret_cast<KeyPressed*>(p_event);
				//TRC_INFO("key: %c", press->m_keycode);
				break;
			}
			case trace::EventType::TRC_KEY_RELEASED:
			{
				KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);
				//TRC_DEBUG("key: %c", release->m_keycode);
				if (release->m_keycode == Keys::KEY_ESCAPE)
				{
					m_isRunning = false;
				}
				break;
			}

			case EventType::TRC_WND_RESIZE:
			{
				WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
				//TRC_WARN("Width: %d", wnd->m_width);
				//TRC_ERROR("Height: %d", wnd->m_height);
				break;
			}

			case EventType::TRC_BUTTON_PRESSED:
			{
				MousePressed* mouse = reinterpret_cast<MousePressed*>(p_event);
				TRC_CRITICAL("Button: %d", mouse->m_button);
				break;
			}

			case EventType::TRC_BUTTON_RELEASED:
			{
				MouseReleased* mouse = reinterpret_cast<MouseReleased*>(p_event);
				TRC_TRACE("Button: %d", mouse->m_button);
				break;
			}

			case EventType::TRC_MOUSE_MOVE:
			{
				MouseMove* mouse = reinterpret_cast<MouseMove*>(p_event);
				//TRC_WARN("X: %f", mouse->m_x);
				//TRC_WARN("Y: %f", mouse->m_y);
				break;
			}
			}
		}
	}

}