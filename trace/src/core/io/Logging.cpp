#include "pch.h"
#include "Logging.h"
#include "../platform/Windows/WinConsole.h"

namespace trace {

	Exception::Exception(int line, std::string errdesc, std::string function, std::string file)
		: Line(line), mErrDesc(errdesc), mFunction(function), mFile(file)
	{

		std::stringstream Buffer;

		Buffer << "File : " << mFile << "\n";
		Buffer << "Function : " << mFunction << "\n";
		Buffer << "Line : " << Line << "\n";
		Buffer << "Error : " << mErrDesc << "\n\n";

		mErrText = Buffer.str();


	}

	Exception::~Exception() {}

	const char* Exception::what() const noexcept
	{
		return mErrText.c_str();
	}

	Logger* Logger::s_instance = nullptr;

	
	

	void Logger::set_log_level(LogLevel logLevel)
	{
		log_level = logLevel;
	}

	void Logger::set_logger_name(const char * LogName)
	{
		logger_name = LogName;
	}

	Logger * Logger::get_instance()
	{
		
		if (s_instance == nullptr)
		{
			s_instance = new Logger();
			
			if (s_instance->m_console == nullptr)
			{

#ifdef TRC_WINDOWS
				s_instance->m_console = new WinConsole();
				s_instance->m_console->SetAttribLevel(ConsoleAttribLevel::Default);
#else
				s_instance->m_console = new TrcConsole();
				s_instance->m_console->SetAttribLevel(ConsoleAttribLevel::Default);
#endif
			}

			return s_instance;
		}

		return s_instance;

	}

	void Logger::shutdown()
	{
		delete s_instance->m_console;
		s_instance->m_console = nullptr;

	}

	

	void Logger::EnableFileLogging()
	{
		if (file_logging)
		{
			Info("file logging already enabled");
			return;
		}

		file_logging = true;

		filepath = "logs.txt";



		fopen_s(&file, filepath, "w");

		if (!file)
		{
			Critical("Unable to open log file");
		}
	}

	void Logger::EnableFileLogging(const char * file_path)
	{
		if (file_logging)
		{
			Info("file logging already enabled");
			return;
		}

		file_logging = true;

		filepath = file_path;


		fopen_s(&file, filepath, "w");

		if (!file)
		{
			Critical("Unable to open log file");
		}
	}

	Logger::Logger()
		:
		m_console(nullptr),
		log_level(LogLevel::trace),
		filepath(""),
		logger_name("default"),
		file(NULL),
		file_logging(false)
	{
		

	}

	Logger::~Logger()
	{
		if (file)
		{
			fclose(file);
		}

	}

}