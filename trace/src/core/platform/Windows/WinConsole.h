#pragma once

#include "pch.h"

#include <core/io/TrcConsole.h>
#include <core/Enums.h>
#include <Windows.h>

namespace trace {

	class WinConsole : public TrcConsole
	{

	public:
		WinConsole();
			

		~WinConsole();


		virtual void Write(const char* msg, ...) override;


		virtual void SetAttribLevel(ConsoleAttribLevel level) override;
		
	private:
		WORD m_defaultAttrib;
		HANDLE m_handle;

	protected:


	};

}
