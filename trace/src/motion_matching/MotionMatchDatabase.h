#pragma once

#include "animation/Animation.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <unordered_map>

namespace trace::MotionMatching {

	struct JointData
	{
		glm::vec3 position;// zl
		//glm::quat rotation;// yr
		glm::vec3 velocity;// zv
		//glm::vec3 angular_velocity;// yw
	};

	struct FeatureData
	{
		std::vector<JointData> joints;
		glm::vec3 root_position; // zp
		//glm::quat root_rotation; // zr
		glm::vec3 root_velocity; // zv
		//glm::vec3 angular_velocity; // yv

		std::vector<glm::vec3> future_root_positions; //zp
		std::vector<glm::quat> future_root_orientation; // zd

		int32_t clip_index = -1;
	};

	class FeatureDatabase
	{

	public:

		bool ExtractPoseData(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton);

	private:
		std::vector<FeatureData> m_poseData;
		std::unordered_map<int32_t, Ref<AnimationClip>> m_animationIndex;

	protected:

	};

}
