#include "pch.h"
#include "Application.h"
#include "io/Logging.h"
#include "Enums.h"
#include "events/Events.h"
#include "events/EventsSystem.h"
#include "platform\GLFW\GLFWwindow.h"
#include "input\Input.h"


namespace trace
{

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
		

		m_Window->SetVsync(true);


		//----------CLIENT--------------//
		m_client_start();
		//______________________________//

		trace::ApplicationStart app_start;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_START, &app_start);
	}
	
	void Application::Run()
	{
		printf("Application Running\n");


		TRC_WARN("Trace Engine %s", "in progress");



		while (m_isRunning)
		{
			
			m_Window->Update(0.0f);
			InputSystem* input = InputSystem::get_instance();

			for (int i = m_LayerStack->Size() - 1; i >= 0; i--)
			{
				Layer* layer = m_LayerStack->m_Layers[i];
				layer->Update(0.0f);
			}
			
			glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glBegin(GL_TRIANGLES);
			glColor3f(0.45f, 0.355f, 0.54f);
			glVertex2f(-0.5f, -0.5f);
			glColor3f(0.85f, 0.655f, 0.54f);
			glVertex2f(0.5f, -0.5f);
			glColor3f(0.75f, 0.355f, 0.54f);
			glVertex2f(0.0f, 0.5f);
			glEnd();

			//------CLIENT-------//

			m_client_update(0.0f);

			//___________________//


			input->Update(0.0f);
		}
	}

	void Application::End()
	{
		trace::ApplicationEnd app_end;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_END, &app_end);


		//------CLIENT-------//

		m_client_end();

		//___________________//


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
			layer->OnEvent(p_event);
		}

		if (!p_event->m_handled) {
			TRC_TRACE("Event: %s", p_event->GetName());
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
				TRC_WARN(app_end->Log());
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
				TRC_INFO("key: %c", press->m_keycode);
				break;
			}
			case trace::EventType::TRC_KEY_RELEASED:
			{
				KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);
				TRC_DEBUG("key: %c", release->m_keycode);
				if (release->m_keycode == Keys::KEY_ESCAPE)
				{
					m_isRunning = false;
				}
				break;
			}

			case EventType::TRC_WND_RESIZE:
			{
				WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
				TRC_WARN("Width: %d", wnd->m_width);
				TRC_ERROR("Height: %d", wnd->m_height);
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
				TRC_CRITICAL("Button: %d", mouse->m_button);
				break;
			}

			case EventType::TRC_MOUSE_MOVE:
			{
				MouseMove* mouse = reinterpret_cast<MouseMove*>(p_event);
				TRC_WARN("X: %f", mouse->m_x);
				TRC_WARN("Y: %f", mouse->m_y);
				break;
			}
			}
		}
	}

}