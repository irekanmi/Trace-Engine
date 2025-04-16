#include "pch.h"

#include "animation/AnimationBlend.h"
#include "animation/AnimationPose.h"
#include "core/Utils.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "render/Transform.h"

#include "glm/glm.hpp"

namespace trace::Animation {



	void BlendPose(Pose* source, Pose* target, Pose* result, float blend_weight)
	{
		Transform& res_root_motion = result->GetRootMotionDelta();
		Transform& src_root_motion = source->GetRootMotionDelta();
		Transform& dst_root_motion = target->GetRootMotionDelta();
		if (blend_weight == 0.0f)
		{
			*result = *source;
			res_root_motion = src_root_motion;
			return;
		}

		if (blend_weight == 1.0f)
		{
			*result = *target;
			res_root_motion = dst_root_motion;
			return;
		}

		std::vector<Transform>& source_pose = source->GetLocalPose();
		std::vector<Transform>& target_pose = target->GetLocalPose();

		for (uint32_t i = 0; i < source_pose.size(); i++)
		{
			Transform result_pose;
			BlendTransform(&source_pose[i], &target_pose[i], &result_pose, blend_weight);
			result->GetLocalPose()[i] = result_pose;
		}

		res_root_motion = Transform::Blend(src_root_motion, dst_root_motion, blend_weight);
	}

	void BlendTransform(Transform* source, Transform* target, Transform* result, float blend_weight)
	{
		glm::vec3 position = trace::lerp(source->GetPosition(), target->GetPosition(), blend_weight);
		glm::quat rotation = trace::slerp(source->GetRotation(), target->GetRotation(), blend_weight);
		glm::vec3 scale = trace::lerp(source->GetScale(), target->GetScale(), blend_weight);

		result->SetPosition(position);
		result->SetRotation(rotation);
		result->SetScale(scale);
	}

}