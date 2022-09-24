#include "pch.h"
#include "Events.h"

namespace trace {

	
	ConsoleWriteEvent::ConsoleWriteEvent()
		{
			m_type = EventType::TRC_CONSOLE_WRITE;
		}
	ConsoleWriteEvent::~ConsoleWriteEvent()
		{

		}

	void ConsoleWriteEvent::SetData(std::string data) { m_data = data; };
	std::string ConsoleWriteEvent::GetData() { return m_data; }

}