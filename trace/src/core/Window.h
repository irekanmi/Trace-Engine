#pragma once

#include "Core.h"
#include "pch.h"
#include "Coretypes.h"


namespace trace {

	class TRACE_API Window
	{
		
	public:

		virtual ~Window() {};

		virtual void Init(const WindowDecl & win_prop) = 0;
		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void ShutDown() = 0;
		virtual void SetVsync(bool enable) = 0;
		virtual void* GetNativeHandle() = 0;

	private:
	protected:
		bool m_isVsync = false;
	};

}