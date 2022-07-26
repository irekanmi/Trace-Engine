#include "pch.h"

#include "Events.h"

namespace trace {

	KeyPressed::KeyPressed(Keys keycode)
	{
		m_type = EventType::TRC_KEY_PRESSED;
		m_keycode = keycode;
	}

	KeyPressed::~KeyPressed()
	{

	}

	KeyReleased::KeyReleased(Keys keycode)
	{
		m_type = EventType::TRC_KEY_RELEASED;
		m_keycode = keycode;
	}

	KeyReleased::~KeyReleased()
	{

	}

	MousePressed::MousePressed(Buttons button)
	{
		m_type = EventType::TRC_BUTTON_PRESSED;
		m_button = button;
	}

	MousePressed::~MousePressed()
	{

	}

	MouseReleased::MouseReleased(Buttons button)
	{
		m_type = EventType::TRC_BUTTON_RELEASED;
		m_button = button;
	}

	MouseReleased::~MouseReleased()
	{

	}

	MouseMove::MouseMove()
	{
		m_type = EventType::TRC_MOUSE_MOVE;
	}

	MouseMove::MouseMove(float x, float y)
	{
		m_type = EventType::TRC_MOUSE_MOVE;
		m_x = x; m_y = y;
	}

	MouseMove::~MouseMove()
	{

	}
}