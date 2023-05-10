#pragma once

#include "core/Core.h"
#include "core/Enums.h"

namespace trace {

	class TRACE_API GHandle
	{

	public:
		GHandle();
		~GHandle();

	private:
		void* m_internalData;
	protected:


	};

}
