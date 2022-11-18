#pragma once

#include "Core.h"

namespace trace {


	class TRACE_API Object
	{
	public:

		Object();
		Object(const char* name);
		virtual ~Object();

		uint32_t GetID() { return m_id; }
		const char* GetName();

	private:


	protected:
		uint32_t m_id;
		const char* m_name;

	};

}
