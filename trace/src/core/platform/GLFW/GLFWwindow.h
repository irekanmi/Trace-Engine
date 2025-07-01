#pragma once

#include "core/Window.h"
#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include "EASTL/string.h"

namespace trace {

	class GLFW_Window : public Window
	{

	public:
		GLFW_Window();
		~GLFW_Window();

		virtual void Init(const WindowDecl& win_prop) override;
		virtual unsigned int GetWidth() override;
		virtual unsigned int GetHeight() override;
		virtual void Update(float deltaTime) override;
		virtual void ShutDown() override;
		virtual void SetVsync(bool enable) override;
		virtual void* GetNativeHandle() override;
		virtual void* GetHandle() override;
		virtual void PollAndUpdateEvents() override;


	private:
		struct WindowData
		{
			unsigned int Width;
			unsigned int Height;
			std::string Name;
		};

		GLFWwindow* m_pWindow;
		WindowData m_Data;

	protected:

	};
	

}
