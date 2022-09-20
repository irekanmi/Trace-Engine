#pragma once

#include "../Core.h"

namespace trace {

	class TRACE_API Console
	{

	public:
		Console();
		virtual ~Console();

		virtual void Write( const char* msg, ... );
		virtual void Trace();
		virtual void Debug();
		virtual void Info();
		virtual void Warn();
		virtual void Error();
		virtual void Critical();


	private:
	protected:


	};

}
