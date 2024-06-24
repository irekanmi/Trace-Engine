#include "pch.h"
#include "EventsSystem.h"
#include "core/Enums.h"

namespace trace {


	EventsSystem::EventsSystem()
		:Object::Object(_STR(EventsSystem))
	{

	}

	EventsSystem::~EventsSystem()
	{

	}

	void EventsSystem::AddEventListener(EventType event_type, EventCallbackFunc func)
	{
		// TODO: check if listener is already added
		m_events[event_type].push_back(func);
	}

	void EventsSystem::DispatchEvent(EventType event_type, Event* p_event)
	{
		//TODO: create a container to hold events "for multithreading"
		for (size_t i = 0; i < m_events[event_type].size(); i++)
		{
			m_events[event_type][i](p_event);
			if (p_event->IsHandled())
			{
				break;
			}
		}

	}

	EventsSystem* EventsSystem::get_instance()
	{
		static EventsSystem* s_instance = new EventsSystem;
		return s_instance;
	}

	void EventsSystem::shutdown()
	{
	}

}