#include "pch.h"
#include "Events.h"

namespace trace {

	ApplicationStart::ApplicationStart()
	{
		m_type = EventType::TRC_APP_START;
	}

	ApplicationStart::~ApplicationStart()
	{

	}

	const char* ApplicationStart::Log()
	{
		return "Application started";
	}


	ApplicationEnd::ApplicationEnd()
	{
		m_type = EventType::TRC_APP_END;
	}

	ApplicationEnd::~ApplicationEnd()
	{

	}

	const char* ApplicationEnd::Log()
	{
		return "Application ended";
	}

	WindowClose::WindowClose()
	{
		m_type = EventType::TRC_WND_CLOSE;
	}

	WindowClose::~WindowClose()
	{

	}

}