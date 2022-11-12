#pragma once

#include "Core.h"
#include "Enums.h"
#include "render/GPUtypes.h"

namespace trace {

	typedef void(*ClientUpdateCallback)(float deltaTime);
	typedef void(*ClientStartCallback)();
	typedef void(*ClientEndCallback)();

	enum WindowType
	{
		GLFW_WINDOW,
		WIN32_WINDOW
	};

	struct WindowDecl
	{

		WindowDecl(std::string name = "Trace Engine",
			unsigned int width = 800,
			unsigned int height = 600)
		{
			m_window_name = name;
			m_width = width;
			m_height = height;
		}

		std::string m_window_name;
		unsigned int m_width;
		unsigned int m_height;
	};

	struct trc_app_data
	{
		trc_app_data() {}
		WindowDecl winprop;
		WindowType wintype;
		RenderAPI graphics_api;
		bool enable_vsync = false;
		bool windowed;
		ClientEndCallback client_end;
		ClientStartCallback client_start;
		ClientUpdateCallback client_update;
	};

}
