#include "pch.h"
#include "Logging.h"
#include "core/Core.h"
#include "core/Enums.h"

namespace trace {

	Exception::Exception(int line, std::string err_desc, std::string function, std::string file)
		: m_line(line), m_errDesc(err_desc), m_function(function), m_file(file)
	{

		std::stringstream buffer;

		buffer << "File : " << m_file << "\n";
		buffer << "Function : " << m_function << "\n";
		buffer << "Line : " << m_line << "\n";
		buffer << "Error : " << m_errDesc << "\n\n";

		m_errText = buffer.str();


	}

	Exception::~Exception() {}

	const char* Exception::what() const noexcept
	{
		return m_errText.c_str();
	}


	
	

	void Logger::SetLogLevel(LogLevel logLevel)
	{
		logger->set_level((spdlog::level::level_enum)logLevel);
		if (file_logging)
		{
			file_logger->set_level((spdlog::level::level_enum)logLevel);
		}
	}

	void Logger::SetLoggerName(const char * LogName)
	{
		logger_name = LogName;
	}

	Logger * Logger::get_instance()
	{
		
		static Logger* s_instance = new Logger;
		return s_instance;

	}

	void Logger::Shutdown()
	{

	}

	

	void Logger::EnableFileLogging()
	{
		if (file_logging)
		{
			Info("file logging already enabled");
			return;
		}

		file_logging = true;

		filepath = "../logs.txt";


		file_logger = spdlog::basic_logger_mt("TRACE_FILE", filepath, true);
		file_logger->set_pattern("[%n][%H:%M:%S:%e]%^[%l] %v%$");

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
		file_logger = spdlog::basic_logger_st("TRACE", filepath, true);
		file_logger->set_pattern("[%n][%H:%M:%S:%e]%^[%l] %v%$");


	}

	Logger::Logger()
		:
		Object(_STR(Logger)),
		filepath(""),
		logger_name("default"),
		file_logging(false)
	{
		
		logger = spdlog::stdout_color_mt("TRACE");
		logger->set_pattern("%^[%n][%H:%M:%S:%e][%l] %v%$");
	}

	Logger::~Logger()
	{
		Shutdown();
	}

}