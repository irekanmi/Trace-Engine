#pragma once

#include <Core.h>
#include <Coretypes.h>
#include <Enums.h>

#include "render/Graphics.h"
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

		void Update(float deltaTime) ;

		void Init() ;
		void ShutDown() ;

	private:
		Win32Data* m_win32data = nullptr;
		void* m_winHandle = nullptr;
	protected:

	};

}
