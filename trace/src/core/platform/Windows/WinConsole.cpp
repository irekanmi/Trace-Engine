#include "pch.h"

#include <core/io/TrcConsole.h>
#include "WinConsole.h"
#include <core/Enums.h>

namespace trace {

		WinConsole::WinConsole()
			:TrcConsole::TrcConsole()
		{


			m_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO con_info;
			GetConsoleScreenBufferInfo(m_handle, &con_info);
			m_defaultAttrib = con_info.wAttributes;

		}

		WinConsole::~WinConsole()
		{

		}

		void WinConsole::Write(const char* msg, ...)
		{
			va_list arg_ptr;

			char out[KB];

			va_start(arg_ptr, msg);
			vsprintf_s(out, KB, msg, arg_ptr);
			va_end(arg_ptr);

			printf("%s", out);

		}

		void WinConsole::SetAttribLevel(ConsoleAttribLevel level)
		{


			if (level == ConsoleAttribLevel::Default)
			{
				::SetConsoleTextAttribute(m_handle, m_defaultAttrib);
			}
			else {
				WORD color[ConsoleAttribLevel::Default] = {
					FOREGROUND_GREEN  | FOREGROUND_BLUE,
					FOREGROUND_RED | FOREGROUND_INTENSITY | FOREGROUND_BLUE,
					FOREGROUND_GREEN,
					FOREGROUND_RED | FOREGROUND_GREEN,
					FOREGROUND_RED,
					BACKGROUND_RED
				};

				::SetConsoleTextAttribute(m_handle, color[level]);
			}
			m_level = level;
		}

	

}