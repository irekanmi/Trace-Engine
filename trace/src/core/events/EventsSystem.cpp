#include "pch.h"
#include "EventsSystem.h"

namespace trace {

	EventsSystem* EventsSystem::s_instance = nullptr;

	EventsSystem::EventsSystem()
		:Object::Object()
	{

	}

	EventsSystem::~EventsSystem()
	{

	}

	void EventsSystem::AddEventListener(EventType event_type, EventCallbackFunc func)
	{
		m_events[event_type].push_back(func);
	}

	void EventsSystem::AddEvent(EventType event_type, Event* p_event)
	{
		for (size_t i = 0; i < m_events[event_type].size(); i++)
		{
			
			
			m_events[event_type][i](p_event);
			if (p_event->m_handled)
			{
				break;
			}
		}

	}

	EventsSystem * EventsSystem::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new EventsSystem();
		}

		return s_instance;
	}

}