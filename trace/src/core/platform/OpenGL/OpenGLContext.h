#pragma once

#include <Core.h>
#include <Coretypes.h>
#include <Enums.h>

#include "render/GPUtypes.h"
#include "render/GContext.h"
#include <Windows.h>

namespace trace {

	class TRACE_API OpenGLContext : public GContext
	{

	public:
		struct Win32Data
		{
			HDC deviceContext;
			HGLRC openGLContext;
		};

	public:
		OpenGLContext();
		~OpenGLContext();

		virtual void Update(float deltaTime) override;

		virtual void Init() override;
		virtual void ShutDown() override;

	private:
		Win32Data* m_win32data = nullptr;
		void* m_winHandle = nullptr;
	protected:

	};

}
