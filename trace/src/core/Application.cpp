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
#include "render/PerspectiveCamera.h"

//// Temp---------------
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
////--------------------

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
	}
	
	void Application::Run()
	{
		printf("Application Running\n");


		TRC_WARN("Trace Engine %s", "in progress");


		

		InputSystem* input = InputSystem::get_instance();
		Renderer* renderer = Renderer::get_instance();
		
		//Temp------------------------
		renderer->Start();
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