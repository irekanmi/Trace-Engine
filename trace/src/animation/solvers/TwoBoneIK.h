#pragma once

#include "trace.h"
#include "animation/AnimationPose.h"


#include <glm/glm.hpp>
#include <array>

namespace trace::Animation {


	struct TwoBoneIKData
	{
		Animation::Pose* anim_pose = nullptr;
		uint32_t root_bone_index;
		uint32_t mid_bone_index;
		uint32_t end_bone_index;
		glm::vec3 target_position;
		glm::vec3 pole_hint;
		bool use_hint = false;
	};

	class TwoBoneIK
	{

	public:

		static void Solve(TwoBoneIKData& ik_data, std::array<glm::quat, 2>& out_orientation);

	private:
	protected:


	};

}

