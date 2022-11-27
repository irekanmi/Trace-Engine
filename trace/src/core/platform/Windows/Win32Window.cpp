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
#endif

static LRESULT CALLBACK win_proc(HWND wnd, uint32_t msg, WPARAM wparam, LPARAM lparam);

static eastl::wstring to_ws(const eastl::string& str);


namespace trace {

	trace::Win32Window::Win32Window(const WindowDecl& win_prop)
	{
		Init(win_prop);
	}

	trace::Win32Window::~Win32Window()
	{
		ShutDown();
	}

	void trace::Win32Window::Init(const WindowDecl& win_prop)
	{
		WNDCLASSEX wnd;
		HINSTANCE hinstance = (HINSTANCE)Platform::GetAppHandle();
		Platform::ZeroMem(&wnd, sizeof(WNDCLASSEX));
		wnd.cbSize = sizeof(WNDCLASSEX);
		wnd.hInstance = hinstance;
		wnd.lpszClassName = to_ws(_NAME_).c_str();
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

		uint32_t _width = win_prop.m_width;
		uint32_t _height = win_prop.m_height;
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
			to_ws(win_prop.m_window_name).c_str(),
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
		m_data.width = win_prop.m_width;
		m_data.height = win_prop.m_height;

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

		MSG msg;

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for (int i = 0; i < Keys::KEYS_MAX_KEYS; i++)
		{
			InputSystem::get_instance()->SetKey((Keys)i, GetAsyncKeyState(i) & 0x8000f);
		}
		for (int i = 0; i < 5; i++)
		{
			InputSystem::get_instance()->SetButton((Buttons)i, GetAsyncKeyState(i) & 0x8000f);
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

}

static LRESULT CALLBACK win_proc(HWND wnd, uint32_t msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
	{
		// Notify the OS that erasing is handled by the application to prevent 
		return 1;
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

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		uint16_t key = (uint16_t)wparam;
		trace::Keys _key = (trace::Keys)key;

		trace::KeyReleased release(_key);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_KEY_RELEASED, &release);


		break;
	}
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

		trace::MouseMove move(xPos, yPos);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_MOUSE_MOVE, &move);

		trace::InputSystem::get_instance()->SetMouseX(xPos);
		trace::InputSystem::get_instance()->SetMouseY(yPos);

		break;
	}

	// TODO: mouse wheel
	case WM_MOUSEWHEEL:
	{
		break;
	}

	case WM_LBUTTONUP:
	{
		trace::MouseReleased release(trace::Buttons::BUTTON_LEFT);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_RELEASED, &release);



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
		break;
	}
	case WM_MBUTTONDOWN:
	{
		trace::MousePressed press(trace::Buttons::BUTTON_MIDDLE);
		trace::EventsSystem::get_instance()->DispatchEvent(trace::EventType::TRC_BUTTON_PRESSED, &press);

		trace::InputSystem::get_instance()->SetButton(trace::Buttons::BUTTON_MIDDLE, 1);

		break;
	}


	}
	
	return DefWindowProc(wnd, msg, wparam, lparam);
}

static eastl::wstring to_ws(const eastl::string& str)
{
	size_t len = mbstowcs(nullptr, &str[0], 0);
	if (len == -1)
	{
		TRC_ASSERT(false, "invalid string");
		return eastl::wstring();
	}
	eastl::wstring ret;
	mbstowcs(&ret[0], &str[0], len + 1);
	return ret;
}
