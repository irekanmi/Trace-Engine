#include "pch.h"

#include "Console.h"
#include "../Enums.h"
#include <stdarg.h>

namespace trace {

	Console::Console()
	{

	}

	Console::~Console()
	{

	}

	void Console::Write( const char* msg, ... )
	{

		va_list arg_ptr;

		va_start(arg_ptr, msg);
		vprintf(msg, arg_ptr);
		va_end(arg_ptr);

	}

	void Console::Trace()
	{
	}

	void Console::Debug()
	{
	}

	void Console::Info()
	{
	}

	void Console::Warn()
	{
	}

	void Console::Error()
	{
	}

	void Console::Critical()
	{
	}
}