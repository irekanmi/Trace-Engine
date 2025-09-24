#pragma once

#include "Keys.h"
#include "core/Object.h"

#include <unordered_map>

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
		float x;
		float y;
	};
	
	struct Gamepad
	{
		bool buttons[GamepadKeys::GAMEPAD_MAX_KEYS + 1] = {0};
		float left_stick_x;
		float left_stick_y;
		float right_stick_x;
		float right_stick_y;
		float left_trigger;
		float right_trigger;

		int32_t controller_id = -1;
	};

	class TRACE_API InputSystem : public Object
	{

	public:
		InputSystem();
		~InputSystem();

		void Update(float deltaTime);

		KeyState GetKeyState(Keys key);
		KeyState GetButtonState(Buttons button);
		KeyState GetGamepadKeyState(GamepadKeys key, int32_t controller_id = 0);

		bool GetKey(Keys key);
		bool GetKeyPressed(Keys key);
		bool GetKeyReleased(Keys key);
		bool GetGamepadKey(GamepadKeys key, int32_t controller_id = 0);
		bool GetGamepadKeyPressed(GamepadKeys key, int32_t controller_id = 0);
		bool GetGamepadKeyReleased(GamepadKeys key, int32_t controller_id = 0);
		bool GetButton(Buttons button);
		bool GetButtonPressed(Buttons button);
		bool GetButtonReleased(Buttons button);

		void SetKey(Keys key, bool value);
		void SetButton(Buttons button, bool value);

		void SetMouseX(float x);
		void SetMouseY(float y);

		float GetMouseX();
		float GetMouseY();
		
		float GetLeftStickX(int32_t controller_id = 0);
		float GetLeftStickY(int32_t controller_id = 0);

		float GetRightStickX(int32_t controller_id = 0);
		float GetRightStickY(int32_t controller_id = 0);

		float GetLeftTrigger(int32_t controller_id = 0);
		float GetRightTrigger(int32_t controller_id = 0);

		static InputSystem* get_instance();
		static InputSystem* s_instance;
	private:
		KeyBoard keyboard_curr;
		KeyBoard keyboard_prev;

		Mouse mouse_curr;
		Mouse mouse_prev;

		std::unordered_map<int32_t, Gamepad> gamepads_curr;
		std::unordered_map<int32_t, Gamepad> gamepads_prev;

	protected:
		friend class Win32Window;

	};
}
