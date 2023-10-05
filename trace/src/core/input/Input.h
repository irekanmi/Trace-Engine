#pragma once

#include "Keys.h"
#include "core/Object.h"

namespace trace {

	enum KeyState
	{
		KEY_NULL,
		KEY_PRESS,
		KEY_RELEASE,
		KEY_HELD,
	};

	struct KeyBoard
	{
		bool keys[Keys::KEYS_MAX_KEYS + 1];
	};

	struct Mouse
	{
		bool buttons[Buttons::BUTTON_MAX_BUTTONS + 1];
		float m_x;
		float m_y;
	};

	class TRACE_API InputSystem : public Object
	{

	public:
		InputSystem();
		~InputSystem();

		void Update(float deltaTime);

		KeyState GetKeyState(Keys key);
		KeyState GetButtonState(Buttons button);

		bool GetKey(Keys key);
		bool GetButton(Buttons button);

		void SetKey(Keys key, bool val);
		void SetButton(Buttons button, bool val);

		void SetMouseX(float x);
		void SetMouseY(float y);

		float GetMouseX();
		float GetMouseY();

		static InputSystem* get_instance();
		static InputSystem* s_instance;
	private:
		KeyBoard keyboard_curr;
		KeyBoard keyboard_prev;

		Mouse mouse_curr;
		Mouse mouse_prev;

	protected:

	};
}
