#include "pch.h"

#include "animation/AnimationBlend.h"
#include "animation/AnimationPose.h"
#include "core/Utils.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

#include "glm/glm.hpp"

namespace trace::Animation {



	void BlendPose(Pose* source, Pose* target, Pose* result, float blend_weight)
	{

		if (blend_weight == 0.0f)
		{
			*result = *source;

			return;
		}

		if (blend_weight == 1.0f)
		{
			*result = *target;

			return;
		}

		std::vector<Transform>& source_pose = source->GetLocalPose();
		std::vector<Transform>& target_pose = target->GetLocalPose();

		for (uint32_t i = 0; i < source_pose.size(); i++)
		{
			glm::vec3 position = trace::lerp(source_pose[i].GetPosition(), target_pose[i].GetPosition(), blend_weight);
			glm::quat rotation = trace::slerp(source_pose[i].GetRotation(), target_pose[i].GetRotation(), blend_weight);
			glm::vec3 scale = trace::lerp(source_pose[i].GetScale(), target_pose[i].GetScale(), blend_weight);

			result->GetLocalPose()[i].SetPosition(position);
			result->GetLocalPose()[i].SetRotation(rotation);
			result->GetLocalPose()[i].SetScale(scale);
		}
	}

}