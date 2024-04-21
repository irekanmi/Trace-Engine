#pragma once

#include "Core.h"
#include "Enums.h"
#include "render/Graphics.h"
#include <string>

namespace trace {

	typedef void(*ClientUpdateCallback)(float deltaTime);
	typedef void(*ClientRenderCallback)(float deltaTime);
	typedef void(*ClientStartCallback)();
	typedef void(*ClientEndCallback)();

	enum WindowType
	{
		DEFAULT,
		GLFW_WINDOW,
		WIN32_WINDOW
	};

	enum PlatformAPI
	{
		NO,
		WINDOWS,
		MACOS,
		LINUX,
		PS4,
		PS5,
		XBOX

	};

	struct WindowDecl
	{

		WindowDecl(std::string name = "Trace",
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
		WindowDecl winprop = WindowDecl();
		WindowType wintype = WindowType::DEFAULT;
		RenderAPI graphics_api = RenderAPI::None;
		PlatformAPI platform_api = PlatformAPI::NO;
		bool enable_vsync = false;
		bool windowed = false;
		ClientEndCallback client_end = nullptr;
		ClientStartCallback client_start = nullptr;
		ClientUpdateCallback client_update = nullptr;
		ClientRenderCallback client_render = nullptr;
		void* render_composer = nullptr;
	};

	class AppSettings
	{
	public:
		static WindowDecl winprop;
		static WindowType wintype;
		static RenderAPI graphics_api;
		static PlatformAPI platform_api;
		static bool enable_vsync;
		static bool windowed;
		static bool is_editor;
		static std::string exe_path;
	};

	

}
