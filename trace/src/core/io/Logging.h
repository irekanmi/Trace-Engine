#pragma once

#include "../Core.h"
#include "../pch.h"
#include "TrcConsole.h"
#include "core/Platform.h"





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
		void Trace(const char * message, Args ...args)
		{
			log(LogLevel::trace, message, args...);
		}

		template<typename ...Args>
		void Debug(const char * message, Args ...args)
		{
			log(LogLevel::debug, message, args...);
		}

		template<typename ...Args>
		void Warn(const char * message, Args ...args)
		{
			log(LogLevel::warn, message, args...);
		}

		template<typename ...Args>
		void Error(const char * message, Args ...args)
		{
			log(LogLevel::error, message, args...);
		}

		template<typename ...Args>
		void Critical(const char * message, Args ...args)
		{
			log(LogLevel::critical, message, args...);
		}

		template<typename ...Args>
		void Info(const char * message, Args ...args)
		{
			log(LogLevel::info, message, args...);
		}

		
		Logger();
		
		~Logger();
		
	private:

		template<typename ...Args>
		void log(LogLevel logLV, const char * msg, Args && ...args)
		{


			ConsoleAttribLevel prev = m_console->GetAttribLevel();

			if (logLV >= log_level)
			{

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

				std::stringstream timeStr;

				tm pTime;
				time_t ctime;
				time(&ctime);

				localtime_s(&pTime, &ctime);

				//char buf[1024]{ 0 };
				Platform::ZeroMem(buf, 1024 * 3);

				sprintf(buf, msg, args...);

				timeStr << "[" << level << "]";
				timeStr << std::setw(2) << "[" << std::setfill('0') << pTime.tm_hour << ":";
				timeStr << std::setw(2) << std::setfill('0') << pTime.tm_min << ":";
				timeStr << std::setw(2) << std::setfill('0') << pTime.tm_sec << "] ";
				timeStr << buf << "\n";

				m_console->SetAttribLevel((ConsoleAttribLevel)logLV);

				//TODO: find a way to write logs in a buffer to be able to send messages to the console in one Write call
				m_console->Write("%s", timeStr.str().c_str());

				m_console->SetAttribLevel(prev);

				if (file)
				{
				//TODO: find a way to write logs in a buffer to be able to send messages to the FILE in one Write call
					fprintf(file, "%s", timeStr.str().c_str());
				}
			}

		}
		
		TrcConsole* m_console;
		LogLevel log_level;
		const char* filepath;
		const char* logger_name;
		FILE* file;
		bool file_logging;
		static Logger* s_instance;
		char* buf;
		

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
#define TRC_TRACE(MSG,...)       trace::Logger::get_instance()->Trace(MSG, __VA_ARGS__);
#define TRC_INFO(MSG,...)        trace::Logger::get_instance()->Info(MSG, __VA_ARGS__);
#define TRC_DEBUG(MSG,...)       trace::Logger::get_instance()->Debug(MSG, __VA_ARGS__);
#define TRC_WARN(MSG,...)        trace::Logger::get_instance()->Warn(MSG, __VA_ARGS__);
#define TRC_ERROR(MSG,...)       trace::Logger::get_instance()->Error(MSG, __VA_ARGS__);
#define TRC_CRITICAL(MSG,...)    trace::Logger::get_instance()->Critical(MSG, __VA_ARGS__);


#ifdef TRC_ASSERT_ENABLED
#ifdef _MSC_VER
#define TRC_ASSERT(exp, MSG_FMT, ...)     \
  if(exp){}                               \
	else{                                 \
TRC_CRITICAL(MSG_FMT, __VA_ARGS__);       \
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
 