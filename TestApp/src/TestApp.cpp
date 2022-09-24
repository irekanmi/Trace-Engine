
#include <trace.h>
#include <stdio.h>
#include <unordered_map>


class TestApp : public  trace::Application
{

public:
	TestApp()
	{
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_START, std::bind(&TestApp::OnEvent, this,std::placeholders::_1));
		trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_APP_END, std::bind(&TestApp::OnEvent, this, std::placeholders::_1));

	}
	~TestApp()
	{

	}

	virtual void Start() override
	{
		trace::ApplicationStart app_start;
		trace::EventsSystem::get_instance()->AddEvent(trace::EventType::TRC_APP_START, &app_start);
	}

	virtual void Run() override
	{
		printf("Application Running\n");


		TRC_TRACE("Trace Engine %s", "in progress");
	

		TRC_INFO("%d", KB);
		TRC_INFO("%d", MB);
		TRC_INFO("%d", GB);

		
	}


	void OnEvent(trace::Event* p_event)
	{
		if (p_event->m_type == trace::EventType::TRC_APP_START)
		{
			trace::ApplicationStart* app_start = reinterpret_cast<trace::ApplicationStart*>(p_event);
			TRC_DEBUG(app_start->Log());
			app_start->m_handled = true;
		}
		if (p_event->m_type == trace::EventType::TRC_APP_END)
		{
			trace::ApplicationEnd* app_end = reinterpret_cast<trace::ApplicationEnd*>(p_event);
			TRC_WARN(app_end->Log());
			app_end->m_handled = true;
		}
	}

	virtual void End() override
	{
		trace::ApplicationEnd app_end;
		trace::EventsSystem::get_instance()->AddEvent(trace::EventType::TRC_APP_END, &app_end);

	}


};


trace::Application* trace::CreateApp()
{

	return new TestApp();

}