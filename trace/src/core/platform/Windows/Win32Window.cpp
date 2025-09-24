#include "pch.h"
#include "Win32Window.h"
#include "core/Platform.h"
#include "core/io/Logging.h"
#include "EASTL/string.h"
#include "core/events/EventsSystem.h"
#include "core/input/Input.h"
#ifdef TRC_WINDOWS
#include <Windows.h>
#include <windowsx.h>
#include <Xinput.h>
#endif

static LRESULT CALLBACK win_proc(HWND wnd, uint32_t msg, WPARAM wparam, LPARAM lparam);

static std::wstring to_ws(const std::string& str);

//NOTE: This is custom defines to allow triggers to have values
#define XINPUT_GAMEPAD_LEFT_TRIGGER 0x1100 
#define XINPUT_GAMEPAD_RIGHT_TRIGGER 0x2200 

namespace trace {

	int gamepadkey_to_xinput(GamepadKeys pad_key)
	{

		switch (pad_key)
		{
		case GamepadKeys::GAMEPAD_BUTTON_1:
		{
			return XINPUT_GAMEPAD_Y;
		}
		case GamepadKeys::GAMEPAD_BUTTON_2:
		{
			return XINPUT_GAMEPAD_B;
		}
		case GamepadKeys::GAMEPAD_BUTTON_3:
		{
			return XINPUT_GAMEPAD_A;
		}
		case GamepadKeys::GAMEPAD_BUTTON_4:
		{
			return XINPUT_GAMEPAD_X;
		}
		case GamepadKeys::GAMEPAD_BUTTON_SELECT:
		{
			return XINPUT_GAMEPAD_BACK;
		}
		case GamepadKeys::GAMEPAD_BUTTON_START:
		{
			return XINPUT_GAMEPAD_START;
		}
		case GamepadKeys::GAMEPAD_DPAD_UP:
		{
			return XINPUT_GAMEPAD_DPAD_UP;
		}
		case GamepadKeys::GAMEPAD_DPAD_DOWN:
		{
			return XINPUT_GAMEPAD_DPAD_DOWN;
		}
		case GamepadKeys::GAMEPAD_DPAD_LEFT:
		{
			return XINPUT_GAMEPAD_DPAD_LEFT;
		}
		case GamepadKeys::GAMEPAD_DPAD_RIGHT:
		{
			return XINPUT_GAMEPAD_DPAD_RIGHT;
		}
		case GamepadKeys::GAMEPAD_STICK_LEFT:
		{
			return XINPUT_GAMEPAD_LEFT_THUMB;
		}
		case GamepadKeys::GAMEPAD_STICK_RIGHT:
		{
			return XINPUT_GAMEPAD_RIGHT_THUMB;
		}
		case GamepadKeys::GAMEPAD_TRIGGER_LEFT:
		{
			return XINPUT_GAMEPAD_LEFT_TRIGGER;
		}
		case GamepadKeys::GAMEPAD_TRIGGER_RIGHT:
		{
			return XINPUT_GAMEPAD_LEFT_TRIGGER;
		}
		case GamepadKeys::GAMEPAD_SHOULDER_LEFT:
		{
			return XINPUT_GAMEPAD_LEFT_SHOULDER;
		}
		case GamepadKeys::GAMEPAD_SHOULDER_RIGHT:
		{
			return XINPUT_GAMEPAD_RIGHT_SHOULDER;
		}
		}

		return -1;
	}

	trace::Win32Window::Win32Window()
	{
	}

	trace::Win32Window::~Win32Window()
	{
		ShutDown();
	}

	void trace::Win32Window::Init(const WindowDecl& window_properties)
	{
		std::wstring class_name = to_ws(_NAME_);
		WNDCLASSEX wnd;
		HINSTANCE hinstance = (HINSTANCE)Platform::GetAppHandle();
		Platform::ZeroMem(&wnd, sizeof(WNDCLASSEX));
		wnd.cbSize = sizeof(WNDCLASSEX);
		wnd.hInstance = hinstance;
		wnd.lpszClassName = class_name.c_str();
		wnd.cbClsExtra = 0;
		wnd.cbWndExtra = 0;
		wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wnd.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wnd.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wnd.lpfnWndProc = win_proc;
		wnd.style = CS_DBLCLKS;
		wnd.lpszMenuName = nullptr;
		
		if (!RegisterClassEx(&wnd))
		{
			MessageBoxA(nullptr, "Failed to Register windows class for window creation", nullptr, IDOK);
			TRC_ERROR("Failed to Register windows class for window creation");
			return;
		}

		uint32_t _width = window_properties.m_width;
		uint32_t _height = window_properties.m_height;
		uint32_t win_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
		uint32_t win_ex_style = WS_EX_APPWINDOW;

		win_style |= WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;

		RECT border = {0};
		AdjustWindowRectEx(&border, win_style, 0, win_ex_style);
		
		_width += border.right - border.left;
		_height += border.bottom - border.top;

		
		
		m_handle = CreateWindowEx(
			win_ex_style,
			to_ws(_NAME_).c_str(),
			to_ws(window_properties.m_window_name).c_str(),
			win_style,
			100,
			100,
			_width,
			_height,
			nullptr,
			nullptr,
			hinstance,
			nullptr
		); 

		if (!m_handle)
		{
			MessageBoxA(nullptr, "Failed to create win32 window", nullptr, IDOK);
			TRC_ERROR("Failed to create win32 window");
			return;
		}
		m_data.width = window_properties.m_width;
		m_data.height = window_properties.m_height;

		// TODO: modify ShowWindow() Check Windows Docs
		ShowWindow(m_handle, SW_SHOW);

		SetWindowLongPtr(m_handle, GWLP_USERDATA, (LONG_PTR)&m_data);
	}

	uint32_t trace::Win32Window::GetWidth()
	{
		return m_data.width;
	}

	uint32_t trace::Win32Window::GetHeight()
	{
		return m_data.height;
	}

	void trace::Win32Window::Update(float deltaTime)
	{

		/*if (!IsWindowEnabled(m_handle))
		{
			TRC_TRACE("Window diabled");
		}*/

		/*for (int i = 0; i < Keys::KEYS_MAX_KEYS; i++)
		{
			InputSystem::get_instance()->SetKey((Keys)i, GetAsyncKeyState(i) & 0x8000f);
		}
		for (int i = 0; i < 5; i++)
		{
			InputSystem::get_instance()->SetButton((Buttons)i, GetAsyncKeyState(i) & 0x8000f);
		}*/

		/*while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}*/

		InputSystem* input_system = InputSystem::get_instance();

		for (int32_t controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++)
		{
			XINPUT_STATE controller_state = { 0 };

			//TODO: Add functions to InputSystem that allow access to gamepad structure
			bool has_controller = input_system->gamepads_curr.find(controller_index) != input_system->gamepads_curr.end();
			if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
			{
				if (!has_controller)
				{
					input_system->gamepads_curr[controller_index] = Gamepad{};
					input_system->gamepads_prev[controller_index] = Gamepad{};
					GamepadConnected controller_connect(controller_index);
					trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_GAMEPAD_CONNECT, &controller_connect);
					TRC_INFO("Controller Connected, Id:{}", controller_index);
				}

				Gamepad& game_controller = input_system->gamepads_curr[controller_index];
				for (int32_t i = 0; i < (int32_t)GamepadKeys::GAMEPAD_MAX_KEYS; i++)
				{
					int win_def = gamepadkey_to_xinput((GamepadKeys)i);
					bool skip = (win_def == XINPUT_GAMEPAD_LEFT_TRIGGER) || (win_def == XINPUT_GAMEPAD_RIGHT_TRIGGER);
					if (skip)
					{
						continue;
					}

					game_controller.buttons[i] = (controller_state.Gamepad.wButtons & win_def) != 0;
				}

				game_controller.buttons[GamepadKeys::GAMEPAD_TRIGGER_LEFT] = controller_state.Gamepad.bLeftTrigger > 5;
				game_controller.buttons[GamepadKeys::GAMEPAD_TRIGGER_RIGHT] = controller_state.Gamepad.bRightTrigger > 5;

				game_controller.left_trigger = (float)controller_state.Gamepad.bLeftTrigger / 255.0f;
				game_controller.right_trigger = (float)controller_state.Gamepad.bRightTrigger / 255.0f;

				game_controller.left_stick_x = fmaxf(-1.0f, (float)controller_state.Gamepad.sThumbLX / 32767.0f);
				game_controller.left_stick_y = fmaxf(-1.0f, (float)controller_state.Gamepad.sThumbLY / 32767.0f);

				game_controller.right_stick_x = fmaxf(-1.0f, (float)controller_state.Gamepad.sThumbRX / 32767.0f);
				game_controller.right_stick_y = fmaxf(-1.0f, (float)controller_state.Gamepad.sThumbRY / 32767.0f);
			}
			else
			{
				if (has_controller)
				{
					GamepadDisconnected controller_disconnect(controller_index);
					trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_GAMEPAD_DISCONNECT, &controller_disconnect);
					input_system->gamepads_curr.erase(controller_index);
					input_system->gamepads_prev.erase(controller_index);
					TRC_INFO("Controller Disconnected, Id:{}", controller_index);
				}
			}
		}
		
	}

	void trace::Win32Window::ShutDown()
	{
		DestroyWindow(m_handle);
	}

	void trace::Win32Window::SetVsync(bool enable)
	{
	}

	void* Win32Window::GetNativeHandle()
	{
		return m_handle;
	}

	void* Win32Window::GetHandle()
	{
		return m_handle;
	}

	void Win32Window::PollAndUpdateEvents()
	{
		if (!IsWindowEnabled(m_handle))
		{
			TRC_TRACE("Window diabled");
		}

		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

}

static LRESULT CALLBACK win_proc(HWND wnd, uint32_t msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	//case WM_PAINT:
	case WM_ERASEBKGND:
	{
		// Notify the OS that erasing is handled by the application to prevent 
		return 0;
	}
	case WM_CLOSE:
	{
		trace::WindowClose wnd_close;
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_WND_CLOSE, &wnd_close);
		return 0;
	}
	case WM_DESTROY:
	{
		//TODO: Check Windows Docs for more info
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE:
	{
		RECT r;
		GetClientRect(wnd, &r);
		uint32_t width = r.right - r.left;
		uint32_t height = r.bottom - r.top;

		trace::WindowResize resize(width, height);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_WND_RESIZE, &resize);

		trace::Win32Window::WindowData* data = (trace::Win32Window::WindowData*)GetWindowLongPtr(wnd, GWLP_USERDATA);
		if (data)
		{
			data->width = width;
			data->height = height;
		}

		break;
	}

	case WM_SHOWWINDOW:
	{
		RECT r;
		GetClientRect(wnd, &r);
		uint32_t width = r.right - r.left;
		uint32_t height = r.bottom - r.top;

		/*trace::WindowResize resize(width, height);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_WND_RESIZE, &resize);

		trace::Win32Window::WindowData* data = (trace::Win32Window::WindowData*)GetWindowLongPtr(wnd, GWLP_USERDATA);
		if (data)
		{
			data->width = width;
			data->height = height;
		}*/

		break;
	}

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		uint16_t key = (uint16_t)wparam;
		trace::Keys _key = (trace::Keys)key;

		trace::KeyReleased release(_key);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_KEY_RELEASED, &release);

		trace::InputSystem::get_instance()->SetKey(_key, 0);
		break;
	}
	
	case WM_CHAR:
	case WM_SYSCHAR:
	{
		uint16_t key = (uint16_t)wparam;
		trace::Keys _key = (trace::Keys)key;

		trace::KeyTyped typed(_key);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_KEY_TYPED, &typed);


		break;
	}
	// TODO : Fix, check why event is still being sent when it is already held
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		uint16_t key = (uint16_t)wparam;
		trace::Keys _key = (trace::Keys)key;

		trace::KeyPressed press(_key);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_KEY_PRESSED, &press);

		trace::InputSystem::get_instance()->SetKey(_key, 1);


		break;
	}


	case WM_MOUSEMOVE:
	{
		int32_t xPos = GET_X_LPARAM(lparam);
		int32_t yPos = GET_Y_LPARAM(lparam);

		trace::MouseMove move(static_cast<float>(xPos), static_cast<float>(yPos));
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_MOUSE_MOVE, &move);

		trace::InputSystem::get_instance()->SetMouseX(static_cast<float>(xPos));
		trace::InputSystem::get_instance()->SetMouseY(static_cast<float>(yPos));

		break;
	}

	case WM_MOUSEWHEEL:
	{
		trace::Win32Window::WindowData* data = (trace::Win32Window::WindowData*)GetWindowLongPtr(wnd, GWLP_USERDATA);

		float zDelta = (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
		float x = 0.0f;
		float y = zDelta;

		trace::MouseWheel mouse_wheel(x, y);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::TRC_MOUSE_WHEEL, &mouse_wheel);

		break;
	}

	case WM_MOUSEHWHEEL:
	{
		trace::Win32Window::WindowData* data = (trace::Win32Window::WindowData*)GetWindowLongPtr(wnd, GWLP_USERDATA);

		float zDelta = (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
		float x = zDelta;
		float y = 0.0f;

		trace::MouseWheel mouse_wheel(x, y);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::TRC_MOUSE_WHEEL, &mouse_wheel);

		break;
	}

	case WM_LBUTTONUP:
	{
		trace::MouseReleased release(trace::Buttons::BUTTON_LEFT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_RELEASED, &release);


		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_LEFT, 0);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		trace::MousePressed press(trace::Buttons::BUTTON_LEFT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_PRESSED, &press);

		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_LEFT, 1);


		break;
	}

	case WM_RBUTTONUP:
	{
		trace::MouseReleased release(trace::Buttons::BUTTON_RIGHT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_RELEASED, &release);

		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_RIGHT, 0);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		trace::MousePressed press(trace::Buttons::BUTTON_RIGHT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_PRESSED, &press);

		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_RIGHT, 1);

		break;
	}

	case WM_MBUTTONUP:
	{
		trace::MouseReleased release(trace::Buttons::BUTTON_MIDDLE);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_RELEASED, &release);

		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_MIDDLE, 0);
		break;
	}
	case WM_MBUTTONDOWN:
	{
		trace::MousePressed press(trace::Buttons::BUTTON_MIDDLE);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_PRESSED, &press);

		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_MIDDLE, 1);

		break;
	}

	case WM_LBUTTONDBLCLK:
	{
		trace::MouseDBClick click(trace::Buttons::BUTTON_LEFT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_MOUSE_DB_CLICK, &click);
		break;
	}

	case WM_RBUTTONDBLCLK:
	{
		trace::MouseDBClick click(trace::Buttons::BUTTON_RIGHT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_MOUSE_DB_CLICK, &click);
		break;
	}

	case WM_MBUTTONDBLCLK:
	{
		trace::MouseDBClick click(trace::Buttons::BUTTON_MIDDLE);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_MOUSE_DB_CLICK, &click);
		break;
	}


	}
	
	return DefWindowProc(wnd, msg, wparam, lparam);
}

static std::wstring to_ws(const std::string& str)
{
	size_t len;// = mbstowcs(nullptr, str.data(), 0);
	mbstowcs_s(&len, nullptr, 0,str.data(), _TRUNCATE);
	if (len == -1)
	{
		TRC_ASSERT(false, "invalid string");
		return std::wstring();
	}
	std::wstring ret;
	ret.reserve(len);
	//mbstowcs(&ret[0], &str[0], len + 1);
	mbstowcs_s(&len, ret.data(), len, str.data(), _TRUNCATE);
	return ret;
}
