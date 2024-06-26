#pragma once

#include "core/Core.h"
#include "core/Coretypes.h"
#include "EASTL/vector.h"

namespace trace {

	// Provide plaform specific functions
	class TRACE_API Platform
	{

	public:

		static void OpenDir();
		static void* GetAppHandle();
		static void ZeroMem(void* dst, uint32_t lenght);
		static void Sleep(float milleseconds);
		static void GetExtensions(uint32_t& count, eastl::vector<const char*>& extensions);
		

	private:
	protected:


	};

}
