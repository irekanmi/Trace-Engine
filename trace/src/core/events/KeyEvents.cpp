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
	
	KeyTyped::KeyTyped(Keys keycode)
	{
		m_type = EventType::TRC_KEY_TYPED;
		m_keycode = keycode;
	}

	KeyTyped::~KeyTyped()
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

	MouseDBClick::MouseDBClick(Buttons button)
	{
		m_type = EventType::TRC_MOUSE_DB_CLICK;
		m_button = button;
	}

	MouseDBClick::~MouseDBClick()
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

	MouseWheel::MouseWheel()
	{
		m_type = EventType::TRC_MOUSE_WHEEL;
	}

	MouseWheel::MouseWheel(float x, float y)
	{
		m_type = EventType::TRC_MOUSE_WHEEL;
		m_x = x; m_y = y;
	}

	MouseWheel::~MouseWheel()
	{

	}
	
	GamepadConnected::GamepadConnected(int32_t controller_id)
	{
		m_type = EventType::TRC_GAMEPAD_CONNECT;
		m_controllerID = controller_id;
	}

	GamepadConnected::~GamepadConnected()
	{

	}
	
	GamepadDisconnected::GamepadDisconnected(int32_t controller_id)
	{
		m_type = EventType::TRC_GAMEPAD_DISCONNECT;
		m_controllerID = controller_id;
	}

	GamepadDisconnected::~GamepadDisconnected()
	{

	}
}