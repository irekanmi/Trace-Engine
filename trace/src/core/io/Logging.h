#pragma once

#include <exception>
#include "../Core.h"
#include <string>
#include <sstream>
#include <mutex>


namespace trace {

	class TRACE_API Logger* s_logger;
	typedef enum {

		trace,
		debug,
		info,
		warn,
		error,
		critical,
		log_none



	} LogLevel;



	class TRACE_API Logger
	{

	public:
		void set_log_level(LogLevel logLevel)
		{
			log_level = logLevel;
		}

		void set_log_name(const char* LogName)
		{
			log_name = LogName;
		}

		static Logger* get_instance()
		{

			return s_logger;

		}

		static void init()
		{

			s_logger = new Logger();

			s_logger->set_log_level(LogLevel::info);

		}

		void EnableFileLogging()
		{
			if (file_logging)
			{
				Info("file logging already enabled");
				return;
			}

			file_logging = true;

			filepath = "logs.txt";

			file = fopen(filepath, "w");

			if (!file)
			{
				Critical("Unable to open log file");
			}
		}

		void EnableFileLogging(const char* file_path)
		{
			if (file_logging)
			{
				Info("file logging already enabled");
				return;
			}

			file_logging = true;

			filepath = file_path;

			file = fopen(filepath, "w");

			if (!file)
			{
				Critical("Unable to open log file");
			}
		}

		template<typename ...Args>
		void Trace(const char* message, Args... args)
		{
			log(LogLevel::trace, message, args...);
		}

		template<typename ...Args>
		void Debug(const char* message, Args... args)
		{
			log(LogLevel::debug, message, args...);
		}

		template<typename ...Args>
		void Info(const char* message, Args... args)
		{
			log(LogLevel::info, message, args...);
		}

		template<typename ...Args>
		void Warn(const char* message, Args... args)
		{
			log(LogLevel::warn, message, args...);
		}

		template<typename ...Args>
		void Error(const char* message, Args... args)
		{
			log(LogLevel::error, message, args...);
		}

		template<typename ...Args>
		void Critical(const char* message, Args... args)
		{
			log(LogLevel::critical, message, args...);
		}

		Logger()
			:log_level(LogLevel::trace)
		{
			file = NULL;
			file = false;
			log_name = "default";
		}

		~Logger()
		{
			fclose(file);

		}
	private:

		template<typename ...Args>
		void log(LogLevel logLV, const char* msg, Args... args)
		{

			if (logLV >= log_level)
			{
				log_mutex.lock();

				const char* level = "Unknow";

				switch (logLV)
				{

				case LogLevel::trace:
					level = "Trace";
					break;

				case LogLevel::debug:
					level = "Debug";
					break;

				case LogLevel::info:
					level = "Info";
					break;

				case LogLevel::warn:
					level = "Warn";
					break;

				case LogLevel::error:
					level = "Error";
					break;

				case LogLevel::critical:
					level = "Critical";
					break;

				}

				printf("[%s] :: %s ->", level, log_name);
				printf(msg, args...);
				printf("\n");

				if (file)
				{
					fprintf(file, "[%s]  :: ", level);
					fprintf(file, msg, args...);
					fprintf(file, "\n");
				}
				log_mutex.unlock();
			}

		}

		LogLevel log_level;
		const char* filepath;
		const char* log_name;
		FILE* file;
		std::mutex log_mutex;
		bool file_logging;


	};


	class TRACE_API Exception : public std::exception
	{

	public:
		Exception(int line, std::string errdesc, std::string function, std::string file);
		~Exception();

		const char* what() const override;

		int Line;
		std::string mErrDesc, mFunction, mFile, mErrText;



	};

}

#define TRC_EXCEPTION(ERR) throw trace::Exception(__LINE__, ERR, __FUNCTION__,__FILE__);
#define TRC_TRACE(MSG,...) trace::Logger::get_instance()->Trace(MSG, __VA_ARGS__);
#define TRC_DEBUG(MSG,...) trace::Logger::get_instance()->Debug(MSG, __VA_ARGS__);
#define TRC_INFO(MSG,...) trace::Logger::get_instance()->Info(MSG, __VA_ARGS__);
#define TRC_WARN(MSG,...) trace::Logger::get_instance()->Warn(MSG, __VA_ARGS__);
#define TRC_ERROR(MSG,...) trace::Logger::get_instance()->Error(MSG, __VA_ARGS__);
#define TRC_CRITICAL(MSG,...) trace::Logger::get_instance()->Critical(MSG, __VA_ARGS__);


