#pragma once

#include "Core.h"

namespace trace {

	class TRACE_API Object
	{
	public:

		Object();
		virtual ~Object();

		unsigned int GetID() { return m_id; }
		void SetID(unsigned int id) { m_id = id; }

	private:


	protected:
		unsigned int m_id;

	};

}
