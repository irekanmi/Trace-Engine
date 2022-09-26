#include "pch.h"
#include "Application.h"
#include "io/Logging.h"
#include "Enums.h"
#include "events/Events.h"
#include "events/EventsSystem.h"
#include "platform\GLFW\GLFWwindow.h"


namespace trace
{

	Application::Application()
	{
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_START, BIND_EVENT_FN(Application::OnEvent));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_END, BIND_EVENT_FN(Application::OnEvent));
	}

	Application::Application(WindowDecl WinProp, WindowType WinType)
	{
		switch (WinType)
		{
		case WindowType::GLFW_WINDOW:
		{
			this->m_Window = new GLFW_Window(WinProp);
			break;
		}
		}
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_START, BIND_EVENT_FN(Application::OnEvent));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_END, BIND_EVENT_FN(Application::OnEvent));
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


		TRC_TRACE("Trace Engine %s", "in progress");


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
		}
	}

}