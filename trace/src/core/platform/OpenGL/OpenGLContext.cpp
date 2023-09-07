#include <pch.h>

#include "OpenGLContext.h"
#include "GL/glew.h"
#include "core/Application.h"
#include "core/io/Logging.h"
#include "GLFW/glfw3.h"
#include "core/Platform.h"

namespace trace {



	OpenGLContext::OpenGLContext()
	{
	}

	OpenGLContext::~OpenGLContext()
	{

	}

	void OpenGLContext::Update(float deltaTime)
	{
		switch (AppSettings::wintype)
		{
		case WindowType::GLFW_WINDOW:
		{
			glfwSwapBuffers((GLFWwindow*)Application::get_instance()->GetWindow()->GetHandle());
			break;
		}
		case WindowType::WIN32_WINDOW:
		{
			SwapBuffers(m_win32data->deviceContext);
			break;
		}
		}

	}

	void OpenGLContext::Init()
	{
		m_winHandle = Application::get_instance()->GetWindow()->GetNativeHandle();
		switch (AppSettings::wintype)
		{
		case WindowType::GLFW_WINDOW:
		{
			if (glewInit() != GLEW_OK)
			{
				TRC_ERROR("Unable to create an OpenGL context glewInit()");
				TRC_ASSERT(false, "glew failed");
				return;
			}
			break;
		}
		case WindowType::WIN32_WINDOW:
		{
			m_win32data = new Win32Data;
			HWND hwnd = (HWND)m_winHandle;
			m_win32data->deviceContext = GetDC(hwnd);
			int format;

			PIXELFORMATDESCRIPTOR pfd;
			Platform::ZeroMem(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
			pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW |
				PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 24;
			pfd.cDepthBits = 16;
			pfd.iLayerType = PFD_MAIN_PLANE;

			format = ChoosePixelFormat(m_win32data->deviceContext, &pfd);

			SetPixelFormat(m_win32data->deviceContext, format, &pfd);

			m_win32data->openGLContext = wglCreateContext(m_win32data->deviceContext);
			
			wglMakeCurrent(m_win32data->deviceContext, m_win32data->openGLContext);

			if (glewInit() != GLEW_OK)
			{
				TRC_ERROR("Unable to create an OpenGL context glewInit()");
				TRC_ASSERT(false, "glew failed");
			}

			break;
		}
		}
	}

	void OpenGLContext::ShutDown()
	{
		if (m_win32data)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(m_win32data->openGLContext);
			ReleaseDC((HWND)m_winHandle, m_win32data->deviceContext);


			delete m_win32data;
		}
	}


}