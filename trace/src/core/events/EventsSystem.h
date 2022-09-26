#pragma once

#include "Events.h"

namespace trace {

	class TRACE_API EventsSystem : public Object
	{

	public:
		EventsSystem();
		~EventsSystem();

		void AddEventListener(EventType event_type, EventCallbackFunc func);
		void AddEvent(EventType event_type, Event* p_event);

		static EventsSystem* s_instance;
		static EventsSystem* get_instance();
		static void shutdown();

	private:
		std::unordered_map<EventType, std::vector<EventCallbackFunc>> m_events;

	protected:

	};

}