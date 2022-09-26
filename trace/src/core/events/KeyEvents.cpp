#include "pch.h"

#include "Events.h"

namespace trace {

	KeyPressed::KeyPressed(int keycode)
	{
		m_type = EventType::TRC_KEY_PRESSED;
		m_keycode = keycode;
	}

	KeyPressed::~KeyPressed()
	{

	}

	KeyReleased::KeyReleased(int keycode)
	{
		m_type = EventType::TRC_KEY_RELEASED;
		m_keycode = keycode;
	}

	KeyReleased::~KeyReleased()
	{

	}
}