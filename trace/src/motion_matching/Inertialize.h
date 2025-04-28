#pragma once

#include "animation/AnimationPose.h"

#include <vector>
#include <array>
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "orange_duck/vec.h"
#include "orange_duck/quat.h"

namespace trace::MotionMatching {


	class Inertialize
	{

	public:

		bool Initialize(Animation::Skeleton* skeleton);
		bool Init(Animation::Pose* source_prev_pose, Animation::Pose* source_pose, Animation::Pose* target_prev_pose, Animation::Pose* target_pose, Animation::Skeleton* skeleton, float animation_dt);
		bool Update(float deltaTime, Animation::Pose* out_pose, Animation::Pose* target_pose, Animation::Skeleton* skeleton);

	private:

	private:
		std::vector<orange_duck::vec3> m_offsetPosition;
		std::vector<orange_duck::vec3> m_offsetVelocity;
		std::vector<orange_duck::quat> m_offsetRotation;
		std::vector<orange_duck::vec3> m_offsetAngularVelocity;
		

	protected:

	};

}
