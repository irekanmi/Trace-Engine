#pragma once

#include "core/Coretypes.h"
#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"
#include <string>

namespace trace::Animation {


	class Bone
	{

	public:

		bool Create(const std::string& name, glm::mat4 bind_pose, glm::mat4 bone_offset);
		void Destory();

		std::string& GetBoneName() { return m_name; }
		glm::mat4 GetBindPose() { return m_bindPose; }
		void SetBindPose(glm::mat4 bind_pose) { m_bindPose = bind_pose; }
		glm::mat4 GetBoneOffset() { return m_boneOffset; }
		StringID GetStringID() { return m_id; }

		int32_t GetParentIndex() { return m_parent; }
		void SetParentIndex(int32_t index) { m_parent = index; }


	private:
		glm::mat4 m_bindPose;
		glm::mat4 m_boneOffset;
		std::string m_name;
		StringID m_id;
		int32_t m_parent = -1;

	protected:
		ACCESS_CLASS_MEMBERS(Bone);

	};

}
