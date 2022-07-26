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

	Application::Application(trc_app_data appData)
	{
		switch (appData.wintype)
		{
		case WindowType::GLFW_WINDOW:
		{
			this->m_Window = new GLFW_Window(appData.winprop);
			break;
		}
		}

		for (unsigned int i = 1; i < EventType::MAX_EVENTS; i++)
		{
			trace::EventsSystem::get_instance()->DispatchEventListener((EventType)i, BIND_EVENT_FN(Application::OnEvent));
		}
	}

	Application::~Application()
	{

	}

	void Application::Start()
	{
		trace::ApplicationStart app_start;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_START, &app_start);

		m_Window->SetVsync(true);
	}
	
	void Application::Run()
	{
		printf("Application Running\n");


		TRC_WARN("Trace Engine %s", "in progress");


		TRC_INFO("%d", KB);
		TRC_INFO("%d", MB);
		TRC_INFO("%d", GB);

		while (m_isRunning)
		{
			
			m_Window->Update(0.0f);

			InputSystem* input = InputSystem::get_instance();

			KeyState key_state = input->GetKeyState(Keys::KEY_A);

			switch (key_state)
			{
			case KeyState::KEY_PRESS:
			{
				printf("Press\n");
				break;
			}
			case KeyState::KEY_HELD:
			{
				printf("Held\n");
			}
			case KeyState::KEY_RELEASE:
			{
				printf("Release\n");
				break;
			}
			}


			InputSystem::get_instance()->Update(0.0f);


		}
	}

	void Application::End()
	{
		trace::ApplicationEnd app_end;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_APP_END, &app_end);

		SAFE_DELETE(m_Window, Window);
	}

	void Application::OnEvent(Event* p_event)
	{
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
			TRC_INFO("key: %d", press->m_keycode);
			break;
		}
		case trace::EventType::TRC_KEY_RELEASED:
		{
			KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);
			TRC_DEBUG("key: %d",release->m_keycode);
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