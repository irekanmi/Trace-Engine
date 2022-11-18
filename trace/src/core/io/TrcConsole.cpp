#include "pch.h"
#include <stdarg.h>

#include "TrcConsole.h"
#include "core/Enums.h"

namespace trace {



	TrcConsole::TrcConsole()
		:Object::Object(_STR(TrcConsole))
	{
	}

	TrcConsole::~TrcConsole()
	{

	}

	void TrcConsole::Write(const char* msg, ...)
	{

		va_list arg_ptr;

		va_start(arg_ptr, msg);
		vprintf(msg, arg_ptr);
		va_end(arg_ptr);


	}

	void TrcConsole::SetAttribLevel(ConsoleAttribLevel level)
	{
		m_level = level;
	}

	ConsoleAttribLevel TrcConsole::GetAttribLevel()
	{
		return m_level;
	}

}