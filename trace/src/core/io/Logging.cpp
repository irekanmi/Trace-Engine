#include "pch.h"
#include "Logging.h"
#include "core/Core.h"
#include "core/Enums.h"

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
		logger->set_level((spdlog::level::level_enum)logLevel);
		if (file_logging)
		{
			file_logger->set_level((spdlog::level::level_enum)logLevel);
		}
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

		}

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
		file_logger = spdlog::basic_logger_mt("TRACE", filepath, true);
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