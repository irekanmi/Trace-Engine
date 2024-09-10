#include "pch.h"

#include "Bone.h"
#include "core/io/Logging.h"

namespace trace {
	bool Bone::Create(const std::string& name, glm::mat4 bind_pose)
	{
		if (!name.empty())
		{
			TRC_WARN("Bone has already been created. Actual name: {}, New name: {}", m_name, name);
			return false;
		}

		m_name = name;
		m_bindPose = bind_pose;

		return true;
	}
	void Bone::Destory()
	{
		m_name.clear();
		m_bindPose = glm::mat4(1.0f);
	}
}