#pragma once

#include "../Core.h"
#include "../Object.h"

namespace trace {


	typedef enum {
		
		Trace,
		Debug,
		Info,
		Warn,
		Error,
		Critical,
		Default

	} ConsoleAttribLevel;


	class TRACE_API TrcConsole : public Object
	{

	public:
		TrcConsole();
		virtual ~TrcConsole();

		virtual void Write( const char* msg, ... );
		virtual void SetAttribLevel(ConsoleAttribLevel level);
		virtual ConsoleAttribLevel GetAttribLevel();


	private:

	protected:
		ConsoleAttribLevel m_level;


	};

}
