#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "core/Coretypes.h"
#include "core/Window.h"


namespace trace {

	class TRACE_API Win32Window : public Window
	{

	public:
		Win32Window(const WindowDecl& win_prop);
		~Win32Window();

		virtual void Init(const WindowDecl& win_prop) override;
		virtual unsigned int GetWidth() override;
		virtual unsigned int GetHeight() override;
		virtual void Update(float deltaTime) override;
		virtual void ShutDown() override;
		virtual void SetVsync(bool enable) override;
		virtual void* GetNativeHandle() override;

		struct WindowData
		{
			uint32_t width;
			uint32_t height;
		};

	private:
		HWND m_handle = nullptr;
		WindowData m_data = {0};
	protected:

	};

}
