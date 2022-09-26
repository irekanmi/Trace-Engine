#include "pch.h"
#include "Application.h"
#include "io/Logging.h"
#include "Enums.h"
#include "events/Events.h"
#include "events/EventsSystem.h"
#include "platform\GLFW\GLFWwindow.h"


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
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_START, BIND_EVENT_FN(Application::OnEvent));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_END, BIND_EVENT_FN(Application::OnEvent));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_CLOSE, BIND_EVENT_FN(Application::OnEvent));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Application::OnEvent));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{

	}

	void Application::Start()
	{
		trace::ApplicationStart app_start;
		trace::EventsSystem::get_instance()->AddEvent(trace::EventType::TRC_APP_START, &app_start);
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
			glClearColor(0.2f, 0.15f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->Update(0.0f);
		}
	}

	void Application::End()
	{
		trace::ApplicationEnd app_end;
		trace::EventsSystem::get_instance()->AddEvent(trace::EventType::TRC_APP_END, &app_end);

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
			TRC_DEBUG("key: %d", release->m_keycode);
			break;
		}
		}
	}

}