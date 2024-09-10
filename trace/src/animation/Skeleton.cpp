#include "pch.h"

#include "animation/Skeleton.h"
#include "core/io/Logging.h"

namespace trace {
	bool Skeleton::Create(const std::string& name, std::vector<Bone>& bones)
	{
		if (!name.empty())
		{
			TRC_WARN("Skeleton has already been created. Actual name: {}, New name: {}", m_name, name);
			return false;
		}

		m_name = name;
		m_bones = bones;

		return true;
	}
	void Skeleton::Destroy()
	{
	}
}