#include "pch.h"

#include "Resource.h"
#include "core/io/Logging.h"
#include "external_utils.h"

#include <mutex>


namespace trace {
	Resource::Resource()
		: m_refCount(0), m_id(INVALID_ID)
	{
	}
	Resource::~Resource()
	{
	}
	void Resource::Destroy()
	{
		TRC_ASSERT(false, "Ensure Asset Destroy function has been implemented");
	}

	UUID Resource::GetUUID()
	{
		UUID result = GetUUIDFromName(GetName());
		return result;
	
	}
	void Resource::Increment()
	{
		std::lock_guard<SpinLock> guard(m_refLock);

		++m_refCount;
	}
	void Resource::Decrement()
	{
		std::lock_guard<SpinLock> guard(m_refLock);

		--m_refCount;
	}
}