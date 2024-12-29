#include "pch.h"

#include "Bone.h"
#include "core/io/Logging.h"
#include "core/Utils.h"

#include <unordered_map>

namespace trace {
	bool Bone::Create(const std::string& name, glm::mat4 bind_pose, glm::mat4 bone_offset)
	{
		if (!m_name.empty())
		{
			TRC_WARN("Bone has already been created. Actual name: {}, New name: {}", m_name, name);
			return false;
		}

		m_name = name;
		m_bindPose = bind_pose;
		m_boneOffset = bone_offset;
		m_id = STR_ID(name);

		return true;
	}
	void Bone::Destory()
	{
		m_name.clear();
		m_bindPose = glm::mat4(1.0f);
	}
}