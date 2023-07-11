#pragma once

#include "../Core.h"
#include "../pch.h"
#include "TrcConsole.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"





namespace trace {

	typedef enum {

		trace      = ConsoleAttribLevel::Trace ,
		debug      = ConsoleAttribLevel::Debug,
		info       = ConsoleAttribLevel::Info,
		warn       = ConsoleAttribLevel::Warn,
		error      = ConsoleAttribLevel::Error,
		critical   = ConsoleAttribLevel::Critical,
		log_none   = ConsoleAttribLevel::Default

	} LogLevel;



	class TRACE_API Logger : public Object
	{

	public:

		void set_log_level(LogLevel logLevel);
		void set_logger_name(const char* LogName);
		void EnableFileLogging();
		void EnableFileLogging(const char* file_path);
		
		static Logger* get_instance();
		void Shutdown();

		template<typename ...Args>
		void Trace(Args ...args)
		{
			logger->trace(args...);
			if (file_logging)
			{
				file_logger->trace(args...);
			}
		}

		template<typename ...Args>
		void Debug(Args ...args)
		{
			logger->debug(args...);
			if (file_logging)
			{
				file_logger->debug(args...);
			}
		}

		template<typename ...Args>
		void Warn(Args ...args)
		{
			logger->warn(args...);
			if (file_logging)
			{
				file_logger->warn(args...);
			}
		}

		template<typename ...Args>
		void Error(Args ...args)
		{
			logger->error(args...);
		}

		template<typename ...Args>
		void Critical(Args ...args)
		{
			logger->critical(args...);
			if (file_logging)
			{
				file_logger->critical(args...);
			}
		}

		template<typename ...Args>
		void Info(Args ...args)
		{
			logger->info(args...);
			if (file_logging)
			{
				file_logger->info(args...);
			}
		}

		
		Logger();
		
		~Logger();
		
	private:


		const char* filepath;
		const char* logger_name;
		bool file_logging;
		static Logger* s_instance;
		
		//spdlog
		std::shared_ptr<spdlog::logger> logger;
		std::shared_ptr<spdlog::logger> file_logger;
	};


	class TRACE_API Exception : public std::exception
	{

	public:
		Exception(int line, std::string errdesc, std::string function, std::string file);
		~Exception();

		const char* what() const noexcept override ;

		int Line;
		std::string mErrDesc, mFunction, mFile, mErrText;



	};

}

#define TRC_EXCEPTION(ERR) throw trace::Exception(__LINE__, ERR, __FUNCTION__,__FILE__);
#define TRC_TRACE(...)       trace::Logger::get_instance()->Trace(__VA_ARGS__);
#define TRC_INFO(...)        trace::Logger::get_instance()->Info(__VA_ARGS__);
#define TRC_DEBUG(...)       trace::Logger::get_instance()->Debug(__VA_ARGS__);
#define TRC_WARN(...)        trace::Logger::get_instance()->Warn(__VA_ARGS__);
#define TRC_ERROR(...)       trace::Logger::get_instance()->Error( __VA_ARGS__);
#define TRC_CRITICAL(...)    trace::Logger::get_instance()->Critical( __VA_ARGS__);


#ifdef TRC_ASSERT_ENABLED
#ifdef _MSC_VER
#define TRC_ASSERT(exp, ...)     \
  if(exp){}                               \
	else{                                 \
TRC_CRITICAL(__VA_ARGS__);       \
      __debugbreak();                     \
}                                         



#else
#define TRC_ASSERT(exp, MSG_FMT, ...)     \
  if(exp){}                               \
	else{                                 \
TRC_CRITICAL(MSG_FMT, __VA_ARGS__);       \
      __builtin_trap();                   \
}                                         
#endif

#else
#define TRC_ASSERT(exp, ...)

#endif
 