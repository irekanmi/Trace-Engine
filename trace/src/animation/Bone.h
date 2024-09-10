#pragma once

#include "glm/glm.hpp"
#include <string>

namespace trace {

	class Bone
	{

	public:

		bool Create(const std::string& name, glm::mat4 bind_pose);
		void Destory();

		std::string& GetBoneName() { return m_name; }
		glm::mat4 GetBindPose() { return m_bindPose; }


	private:
		glm::mat4 m_bindPose;
		std::string m_name;

	protected:

	};

}
