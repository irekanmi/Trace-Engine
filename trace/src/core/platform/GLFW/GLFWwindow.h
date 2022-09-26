#pragma once

#include "core/Window.h"
#include "GLFW/glfw3.h""

namespace trace {

	class GLFW_Window : public Window
	{

	public:
		GLFW_Window(const WindowDecl& win_prop);
		~GLFW_Window();

		virtual void Init(const WindowDecl & win_prop) override;
		virtual unsigned int GetWidth() override;
		virtual unsigned int GetHeight() override;
		virtual void Update(float deltaTime) override;
		virtual void ShutDown() override;
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
