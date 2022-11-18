#include "pch.h"

#include "Input.h"
#include "core/Enums.h"

namespace trace {

	InputSystem* InputSystem::s_instance = nullptr;

	InputSystem::InputSystem()
		: Object(_STR(InputSystem))
	{
		keyboard_curr = { 0 };
		keyboard_prev = { 0 };

		mouse_curr = { 0 };
		mouse_prev = { 0 };
		
	}

	InputSystem::~InputSystem()
	{
	}

	void InputSystem::Update(float deltaTime)
	{
		keyboard_prev = keyboard_curr;
		mouse_prev = mouse_curr;

		keyboard_curr = { 0 };
		mouse_curr = { 0 };
	}

	KeyState InputSystem::GetKeyState(Keys key)
	{
		if (keyboard_prev.keys[key])
		{
			if (keyboard_curr.keys[key])
			{
				return KeyState::KEY_HELD;
			}
			else
			{
				return KeyState::KEY_RELEASE;
			}
		}
		else
		{
			if (keyboard_curr.keys[key])
			{
				return KeyState::KEY_PRESS;
			}
		}
		return KeyState::KEY_NULL;
	}

	KeyState InputSystem::GetButtonState(Buttons button)
	{
		if (mouse_prev.buttons[button])
		{
			if (mouse_curr.buttons[button])
			{
				return KeyState::KEY_HELD;
			}
			else
			{
				return KeyState::KEY_RELEASE;
			}
		}
		else
		{
			if (mouse_curr.buttons[button])
			{
				return KeyState::KEY_PRESS;
			}
		}
		return KeyState::KEY_NULL;
	}

	void InputSystem::SetKey(Keys key, bool val)
	{
		keyboard_curr.keys[key] = val;
	}

	void InputSystem::SetButton(Buttons button, bool val)
	{
		mouse_curr.buttons[button] = val;
	}

	void InputSystem::SetMouseX(float x)
	{
		mouse_curr.m_x = x;
	}

	void InputSystem::SetMouseY(float y)
	{
		mouse_curr.m_y = y;
	}

	float InputSystem::GetMouseX()
	{
		return mouse_curr.m_x;
	}

	float InputSystem::GetMouseY()
	{
		return mouse_curr.m_y;
	}

	InputSystem * InputSystem::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new InputSystem();
		}
		return s_instance;
	}

}