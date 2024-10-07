#pragma once


#include "glm/glm.hpp"
#include <string>

namespace trace {

	using StringID = size_t;

	class Bone
	{

	public:

		bool Create(const std::string& name, glm::mat4 bind_pose, glm::mat4 bone_offset);
		void Destory();

		std::string& GetBoneName() { return m_name; }
		glm::mat4 GetBindPose() { return m_bindPose; }
		glm::mat4 GetBoneOffset() { return m_boneOffset; }
		StringID GetStringID() { return m_id; }


	private:
		glm::mat4 m_bindPose;
		glm::mat4 m_boneOffset;
		std::string m_name;
		StringID m_id;

	protected:

	};

}
