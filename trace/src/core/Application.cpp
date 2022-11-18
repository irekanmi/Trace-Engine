#include "pch.h"
#include "Application.h"
#include "io/Logging.h"
#include "Enums.h"
#include "events/Events.h"
#include "events/EventsSystem.h"
#include "platform\GLFW\GLFWwindow.h"
#include "input\Input.h"
#include "render/Renderer.h"


void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new char[size];
}
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
		}

		m_client_start = appData.client_start;
		m_client_update = appData.client_update;
		m_client_end = appData.client_end;

		m_Window->SetVsync(appData.enable_vsync);

		for (unsigned int i = 1; i < EventType::MAX_EVENTS; i++)
		{
			trace::EventsSystem::get_instance()->AddEventListener((EventType)i, BIND_EVENT_FN(Application::OnEvent));
		}

		eastl::vector<Vertex> verts = {
			{ {-0.5f, -0.5f, 0.0f}, { 0.85f, 0.55f, 0.75f }},
			{ {0.5f, -0.5f, 0.0f}, {0.55f, .75f, .85f} },
			{ {0.0f, 0.5f, 0.0f}, {.75f, .85f, .55f} }
		};

		m_vertices = verts;


		eastl::vector<uint32_t> indices = {
			0, 1, 2
		};

		m_indices = indices;

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
				FragColor = vec4(_color, 1.0f);
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

	}
	
	void Application::Run()
	{
		printf("Application Running\n");


		TRC_WARN("Trace Engine %s", "in progress");


		

		InputSystem* input = InputSystem::get_instance();
		Renderer* renderer = Renderer::get_instance();
		

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

			renderer->BeginScene();
			
			glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glUseProgram(Shader);

			renderer->Draw(VertexBuffer, BufferUsage::VERTEX_BUFFER);

			renderer->EndScene();

			


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


		SAFE_DELETE(m_Window, Window);
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
				//TRC_CRITICAL("Button: %d", mouse->m_button);
				break;
			}

			case EventType::TRC_BUTTON_RELEASED:
			{
				MouseReleased* mouse = reinterpret_cast<MouseReleased*>(p_event);
				//TRC_CRITICAL("Button: %d", mouse->m_button);
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