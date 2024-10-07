#include "pch.h"

#include "Resource.h"
#include "core/io/Logging.h"


namespace trace {
	Resource::Resource()
	{
	}
	Resource::~Resource()
	{
	}
	void Resource::Destroy()
	{
		TRC_ASSERT(false, "Ensure Asset Destroy function has been implemented");
	}
}