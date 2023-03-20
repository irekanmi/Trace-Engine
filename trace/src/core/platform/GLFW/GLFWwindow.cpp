#include "pch.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFWwindow.h"
#include "core/io/Logging.h"
#include "core/events/Events.h"
#include "core/events/EventsSystem.h"
#include "core/input/Input.h"
#include "GLFW/glfw3native.h"
#include "core/Coretypes.h"




namespace trace {

	Keys translateKeyGLFW(int keycode)
	{
		// TODO: Set keys that are not valued yet
		switch (keycode)
		{

		case GLFW_KEY_UP:
			return Keys::KEY_UP;

		case GLFW_KEY_DOWN:
			return Keys::KEY_DOWN;

		case GLFW_KEY_LEFT:
			return Keys::KEY_LEFT;

		case GLFW_KEY_RIGHT:
			return Keys::KEY_RIGHT;

		case GLFW_KEY_ESCAPE:
			return Keys::KEY_ESCAPE;

		case GLFW_KEY_SPACE: 
			return Keys::KEY_SPACE;
		//case GLFW_KEY_APOSTROPHE:  
		case GLFW_KEY_COMMA     :  
			return Keys::KEY_COMMA;
		case GLFW_KEY_MINUS     :  
			return Keys::KEY_MINUS;
		case GLFW_KEY_PERIOD    :  
			return Keys::KEY_PERIOD;
		case GLFW_KEY_SLASH     :   
			return Keys::KEY_SLASH;
		case GLFW_KEY_0  :         
			return Keys::KEY_0;
		case GLFW_KEY_1  :  
			return Keys::KEY_1;
		case GLFW_KEY_2  :    
			return Keys::KEY_2;
		case GLFW_KEY_3  :     
			return Keys::KEY_3;
		case GLFW_KEY_4  :    
			return Keys::KEY_4;
		case GLFW_KEY_5  :       
			return Keys::KEY_5;
		case GLFW_KEY_6  :       
			return Keys::KEY_6;
		case GLFW_KEY_7  :         
			return Keys::KEY_7;
		case GLFW_KEY_8  :    
			return Keys::KEY_8;
		case GLFW_KEY_9  :    
			return Keys::KEY_9;
		case GLFW_KEY_SEMICOLON:
			return Keys::KEY_SEMICOLON;
		case GLFW_KEY_EQUAL    :      
			return Keys::KEY_NUMPAD_EQUAL;
		case GLFW_KEY_A  :            
			return Keys::KEY_A;
		case GLFW_KEY_B  :    
			return Keys::KEY_B;
		case GLFW_KEY_C  :    
			return Keys::KEY_C;
		case GLFW_KEY_D  :    
			return Keys::KEY_D;
		case GLFW_KEY_E  :    
			return Keys::KEY_E;
		case GLFW_KEY_F  :    
			return Keys::KEY_F;
		case GLFW_KEY_G  :    
			return Keys::KEY_G;
		case GLFW_KEY_H  :    
			return Keys::KEY_H;
		case GLFW_KEY_I  :    
			return Keys::KEY_I;
		case GLFW_KEY_J  :    
			return Keys::KEY_J;
		case GLFW_KEY_K  :    
			return Keys::KEY_K;
		case GLFW_KEY_L  :    
			return Keys::KEY_L;
		case GLFW_KEY_M  :    
			return Keys::KEY_M;
		case GLFW_KEY_N  :    
			return Keys::KEY_N;
		case GLFW_KEY_O  :    
			return Keys::KEY_O;
		case GLFW_KEY_P  :    
			return Keys::KEY_P;
		case GLFW_KEY_Q  :    
			return Keys::KEY_Q;
		case GLFW_KEY_R  :    
			return Keys::KEY_R;
		case GLFW_KEY_S  :    
			return Keys::KEY_S;
		case GLFW_KEY_T  :    
			return Keys::KEY_T;
		case GLFW_KEY_U  :     
			return Keys::KEY_U;
		case GLFW_KEY_V  :    
			return Keys::KEY_V;
		case GLFW_KEY_W  :    
			return Keys::KEY_W;
		case GLFW_KEY_X  :    
			return Keys::KEY_X;
		case GLFW_KEY_Y  :    
			return Keys::KEY_Y;
		case GLFW_KEY_Z  :    
			return Keys::KEY_Z;
		//case GLFW_KEY_LEFT_BRACKET :  
		case GLFW_KEY_BACKSLASH    : 
			return Keys::KEY_SLASH;
		//case GLFW_KEY_RIGHT_BRACKET:  
		case GLFW_KEY_GRAVE_ACCENT : 
			return Keys::KEY_GRAVE;
		//case GLFW_KEY_WORLD_1      :  
		//case GLFW_KEY_WORLD_2      :  
		case GLFW_KEY_ENTER       :
			return Keys::KEY_ENTER;
		case GLFW_KEY_TAB         :  
			return Keys::KEY_TAB;
		case GLFW_KEY_BACKSPACE   :  
			return Keys::KEY_BACKSPACE;
		case GLFW_KEY_INSERT      :   
			return Keys::KEY_INSERT;
		case GLFW_KEY_DELETE      :    
			return Keys::KEY_DELETE;
		//case GLFW_KEY_PAGE_UP     :   
		//case GLFW_KEY_PAGE_DOWN   :   
		case GLFW_KEY_HOME        :   
			return Keys::KEY_HOME;
		case GLFW_KEY_END         :  
			return Keys::KEY_END;
		case GLFW_KEY_CAPS_LOCK   : 
			return Keys::KEY_CAPITAL;
		case GLFW_KEY_SCROLL_LOCK :
			return Keys::KEY_SCROLL;
		case GLFW_KEY_NUM_LOCK    :  
			return Keys::KEY_NUMLOCK;
		case GLFW_KEY_PRINT_SCREEN: 
			return Keys::KEY_PRINT;
		case GLFW_KEY_PAUSE       :
			return Keys::KEY_PAUSE;
		case GLFW_KEY_F1 :  
			return Keys::KEY_F1;
		case GLFW_KEY_F2 :            
			return Keys::KEY_F2;
		case GLFW_KEY_F3 :     
			return Keys::KEY_F3;
		case GLFW_KEY_F4 :     
			return Keys::KEY_F4;
		case GLFW_KEY_F5 :     
			return Keys::KEY_F5;
		case GLFW_KEY_F6 :     
			return Keys::KEY_F6;
		case GLFW_KEY_F7 :     
			return Keys::KEY_F7;
		case GLFW_KEY_F8 :     
			return Keys::KEY_F8;
		case GLFW_KEY_F9 :     
			return Keys::KEY_F9;
		case GLFW_KEY_F10:     
			return Keys::KEY_F10;
		case GLFW_KEY_F11:      
			return Keys::KEY_F11;
		case GLFW_KEY_F12:       
			return Keys::KEY_F12;
		case GLFW_KEY_F13:      
			return Keys::KEY_F13;
		case GLFW_KEY_F14:      
			return Keys::KEY_F14;
		case GLFW_KEY_F15:      
			return Keys::KEY_F15;
		case GLFW_KEY_F16:      
			return Keys::KEY_F16;
		case GLFW_KEY_F17:       
			return Keys::KEY_F17;
		case GLFW_KEY_F18:      
			return Keys::KEY_F18;
		case GLFW_KEY_F19:      
			return Keys::KEY_F19;
		case GLFW_KEY_F20:      
			return Keys::KEY_F20;
		case GLFW_KEY_F21:      
			return Keys::KEY_F21;
		case GLFW_KEY_F22:     
			return Keys::KEY_F22;
		case GLFW_KEY_F23:      
			return Keys::KEY_F23;
		case GLFW_KEY_F24:      
			return Keys::KEY_F24;
		//case GLFW_KEY_F25:            
		case GLFW_KEY_KP_0:
			return Keys::KEY_NUMPAD0;
		case GLFW_KEY_KP_1:
			return Keys::KEY_NUMPAD1;
		case GLFW_KEY_KP_2:
			return Keys::KEY_NUMPAD2;
		case GLFW_KEY_KP_3:
			return Keys::KEY_NUMPAD3;
		case GLFW_KEY_KP_4:
			return Keys::KEY_NUMPAD4;
		case GLFW_KEY_KP_5:
			return Keys::KEY_NUMPAD5;
		case GLFW_KEY_KP_6:
			return Keys::KEY_NUMPAD6;
		case GLFW_KEY_KP_7:
			return Keys::KEY_NUMPAD7;
		case GLFW_KEY_KP_8:
			return Keys::KEY_NUMPAD8;
		case GLFW_KEY_KP_9:         
			return Keys::KEY_NUMPAD9;
		case GLFW_KEY_KP_DECIMAL    : 
			return Keys::KEY_DECIMAL;
		case GLFW_KEY_KP_DIVIDE     : 
			return Keys::KEY_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY   :
			return Keys::KEY_MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT   :
			return Keys::KEY_SUBTRACT;
		case GLFW_KEY_KP_ADD        :
			return Keys::KEY_ADD;
		//case GLFW_KEY_KP_ENTER      : 
		case GLFW_KEY_KP_EQUAL      : 
			return Keys::KEY_NUMPAD_EQUAL;
		case GLFW_KEY_LEFT_SHIFT    : 
			return Keys::KEY_LSHIFT;
		case GLFW_KEY_LEFT_CONTROL  : 
			return Keys::KEY_LCONTROL;
		case GLFW_KEY_LEFT_ALT      : 
			return Keys::KEY_LALT;
		case GLFW_KEY_LEFT_SUPER    : 
			return Keys::KEY_LWIN;
		case GLFW_KEY_RIGHT_SHIFT   : 
			return Keys::KEY_RSHIFT;
		case GLFW_KEY_RIGHT_CONTROL :
			return Keys::KEY_RCONTROL;
		case GLFW_KEY_RIGHT_ALT     :
			return Keys::KEY_RALT;
		case GLFW_KEY_RIGHT_SUPER   :
			return Keys::KEY_RWIN;
		case GLFW_KEY_MENU          :
			// TODO : Verify key
			return Keys::KEY_HOME;
		}

		return Keys::KEYS_MAX_KEYS;
	}

	Buttons translateButtonGLFW(int button)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			return Buttons::BUTTON_LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return Buttons::BUTTON_RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return Buttons::BUTTON_MIDDLE;
		}

		return Buttons::BUTTON_MAX_BUTTONS;
	}


	GLFW_Window::GLFW_Window(const WindowDecl & win_prop)
	{
		Init(win_prop);
	}
	GLFW_Window::~GLFW_Window()
	{
		ShutDown();
	}
	void GLFW_Window::Init(const WindowDecl & win_prop)
	{
		m_Data.Height = win_prop.m_height;
		m_Data.Width = win_prop.m_width;
		m_Data.Name = win_prop.m_window_name;

		int success = glfwInit();
		TRC_ASSERT(success, "glfwInit() Failed");

		switch (AppSettings::graphics_api)
		{
		case RenderAPI::Vulkan:
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			break;
		}
		}

		m_pWindow = glfwCreateWindow(
			m_Data.Width,
			m_Data.Height,
			m_Data.Name.c_str(),
			NULL,
			NULL);

		if (!m_pWindow)
		{
			TRC_ERROR("Unable to create window glfwCreateWindow()");
			return;
		}

		glfwMakeContextCurrent(m_pWindow);
		glfwSetWindowUserPointer(m_pWindow, &m_Data);




		glfwSetWindowCloseCallback(m_pWindow, [](GLFWwindow* window)
		{
			WindowClose wndclose;
			EventsSystem::get_instance()->DispatchEvent(EventType::TRC_WND_CLOSE, &wndclose);
		});

		glfwSetKeyCallback(m_pWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS:
			{
				Keys _key = translateKeyGLFW(key);
				KeyPressed keypress(_key);
				EventsSystem::get_instance()->DispatchEvent(EventType::TRC_KEY_PRESSED, &keypress);
				break;
			}
			case GLFW_RELEASE:
			{
				Keys _key = translateKeyGLFW(key);
				KeyReleased keyrelease(_key);
				EventsSystem::get_instance()->DispatchEvent(EventType::TRC_KEY_RELEASED, &keyrelease);
				break;
			}
			}
		});

		glfwSetWindowSizeCallback(m_pWindow, [](GLFWwindow* window, int width, int height)
		{
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
			data->Width = width;
			data->Height = height;

			WindowResize wnd(width, height);

			EventsSystem::get_instance()->DispatchEvent(EventType::TRC_WND_RESIZE, &wnd);
		});

		glfwSetMouseButtonCallback(m_pWindow, [](GLFWwindow* window, int button, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS:
			{
				Buttons _button = translateButtonGLFW(button);
				InputSystem::get_instance()->SetButton(_button, 1);
				MousePressed mouse(_button);
				EventsSystem::get_instance()->DispatchEvent(EventType::TRC_BUTTON_PRESSED, &mouse);
				break;
			}
			case GLFW_RELEASE:
			{
				Buttons _button = translateButtonGLFW(button);
				MouseReleased mouse(_button);
				EventsSystem::get_instance()->DispatchEvent(EventType::TRC_BUTTON_RELEASED, &mouse);
				break;
			}
			}
		});

		glfwSetCursorPosCallback(m_pWindow, [](GLFWwindow* window, double xpos, double ypos)
		{
			float x = (float)xpos;
			float y = (float)ypos;
			InputSystem::get_instance()->SetMouseX(x);
			InputSystem::get_instance()->SetMouseY(y);
			MouseMove mouse(x, y);
			EventsSystem::get_instance()->DispatchEvent(EventType::TRC_MOUSE_MOVE, &mouse);
		});
	}
	unsigned int GLFW_Window::GetWidth()
	{
		return m_Data.Width;
	}
	unsigned int GLFW_Window::GetHeight()
	{
		return m_Data.Height;
	}
	void GLFW_Window::Update(float deltaTime)
	{

		InputSystem* input = InputSystem::get_instance();
		for (int i = 0; i < GLFW_KEY_LAST + 1; i++)
		{
			Keys _key = translateKeyGLFW(i);
			switch (glfwGetKey(m_pWindow, i))
			{
			case GLFW_PRESS:
			{
				input->SetKey(_key, 1);
				break;
			}
			}	
		}

		for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
		{
			Buttons _button = translateButtonGLFW(i);
			switch (glfwGetMouseButton(m_pWindow, i))
			{
			case GLFW_PRESS:
			{
				input->SetButton(_button, 1);
				break;
			}
			}
		}


		glfwPollEvents();
	}
	void GLFW_Window::ShutDown()
	{
		glfwDestroyWindow(m_pWindow);
	}
	void GLFW_Window::SetVsync(bool enable)
	{
		m_isVsync = enable;
		if (m_isVsync)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
	}
	void* GLFW_Window::GetNativeHandle()
	{
		void* value = nullptr;
		switch (AppSettings::platform_api)
		{
		case PlatformAPI::WINDOWS:
		{
			value = glfwGetWin32Window(m_pWindow);
			break;
		}

		}

		return value;
	}
	void* GLFW_Window::GetHandle()
	{
		return m_pWindow;
	}
}