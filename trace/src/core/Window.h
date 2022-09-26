#pragma once

#include "Core.h"
#include "pch.h"


namespace trace {

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

	
	class TRACE_API Window
	{
		
	public:

		virtual ~Window() {};

		virtual void Init(const WindowDecl & win_prop) = 0;
		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void ShutDown() = 0;
	private:
	protected:
	};

}