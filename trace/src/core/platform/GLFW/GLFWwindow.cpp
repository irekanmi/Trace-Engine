#include "pch.h"

#include "GLFWwindow.h"
#include "core/io/Logging.h"
#include "core/events/Events.h"
#include "core/events/EventsSystem.h"

namespace trace {
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
			EventsSystem::get_instance()->AddEvent(EventType::TRC_WND_CLOSE, &wndclose);
		});

		glfwSetKeyCallback(m_pWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressed keypress(key);
				EventsSystem::get_instance()->AddEvent(EventType::TRC_KEY_PRESSED, &keypress);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleased keyrelease(key);
				EventsSystem::get_instance()->AddEvent(EventType::TRC_KEY_RELEASED, &keyrelease);
				break;
			}
			}
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
		glfwPollEvents();
		glfwSwapBuffers(m_pWindow);

	}
	void GLFW_Window::ShutDown()
	{
		glfwDestroyWindow(m_pWindow);
	}
}