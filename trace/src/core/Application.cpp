#include "pch.h"

#include "Enums.h"
#include "Application.h"
#include "Platform.h"
#include "io/Logging.h"
#include "events/Events.h"
#include "events/EventsSystem.h"
#include "platform/GLFW/GLFWwindow.h"
#include "core/platform/Windows/Win32Window.h"
#include "input/Input.h"
#include "render/Renderer.h"
#include "memory/memory.h"
#include "resource/ResourceSystem.h"
#include "core/memory/MemoryManager.h"

//Temp==================
#include "render/Graphics.h"
#include "glm/glm.hpp"
#include "backends/Renderutils.h"
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
	extern std::vector<trace::Object*> g_SystemPtrs;
	
	
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

	std::vector<Object*> Application::GetEngineSystemsID()
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
			this->m_Window = new GLFW_Window(appData.winprop); // TODO: Use Custom Allocator or Add Window creation tp backend
			break;
		}
		case WindowType::WIN32_WINDOW:
		{
			this->m_Window = new Win32Window(appData.winprop); // TODO: Use Custom Allocator or Add Window creation tp backend
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

		ApplicationStart app_start;
		EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_START, &app_start);

		Renderer* renderer = Renderer::get_instance();
		renderer->Start();
		ResourceSystem::LoadDefaults();

		//----------CLIENT--------------//
		m_client_start();
		//______________________________//
	}
	
	void Application::Run()
	{
		TRC_WARN("Trace Engine {}", "in progress");

		InputSystem* input = InputSystem::get_instance();
		Renderer* renderer = Renderer::get_instance();
		MemoryManager* mem_manager = MemoryManager::get_instance();

		
		m_clock.Begin();

		m_lastTime = m_clock.GetElapsedTime();
		while (m_isRunning)
		{
			float _time = m_clock.GetElapsedTime();
			float deltaTime = _time - m_lastTime;
			m_lastTime = _time;

			mem_manager->BeginFrame();

			m_Window->Update(0.0f);
			for (int i = m_LayerStack->Size() - 1; i >= 0; i--)
			{
				Layer* layer = m_LayerStack->m_Layers[i];
				layer->Update(deltaTime);
			}

			//------CLIENT-------//

			m_client_update(deltaTime);

			//___________________//	

			renderer->Render(deltaTime);

			input->Update(deltaTime);

			mem_manager->EndFrame();

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
				wnd_close->m_handled = true;
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
			}
		}
	}

}