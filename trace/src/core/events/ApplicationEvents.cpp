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

	WindowResize::WindowResize()
	{
		m_type = EventType::TRC_WND_RESIZE;
	}

	WindowResize::WindowResize(unsigned int width, unsigned int height)
	{
		m_type = EventType::TRC_WND_RESIZE;
		m_width = width;
		m_height = height;
	}

	WindowResize::~WindowResize()
	{

	}
}