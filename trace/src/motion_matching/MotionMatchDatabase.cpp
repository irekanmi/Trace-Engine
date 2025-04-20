#include "pch.h"

#include "motion_matching/MotionMatchDatabase.h"
#include "core/io/Logging.h"
#include "animation/AnimationEngine.h"
#include "animation/AnimationPose.h"
#include "scene/Entity.h"
#include "scene/Scene.h"


namespace trace::MotionMatching {



	bool FeatureDatabase::ExtractPoseData(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton)
	{
		if (!clip)
		{
			TRC_ERROR("Invalid Animation Clip Handle, Function: {}", __FUNCTION__);
			return false;
		}

		if (!skeleton)
		{
			TRC_ERROR("Invalid Skeleton Handle, Function: {}", __FUNCTION__);
			return false;
		}

		if (!skeleton->GetHumanoidRig())
		{
			TRC_ERROR("Skeleton doesn't have a humanoid rig  skeleton name: {}, Function: {}", skeleton->GetName(), __FUNCTION__);
			return false;
		}

		if (!clip->HasRootMotion())
		{
			TRC_ERROR("Animation Clip doesn't have root motion animation clip name: {}, Function: {}", clip->GetName(), __FUNCTION__);
			return false;
		}

		//TODO: Check if there is a better way to save animation clip index
		int32_t clip_index = m_poseData.size();
		m_animationIndex[clip_index] = clip;

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Ref<Animation::HumanoidRig> rig = skeleton->GetHumanoidRig();

		int32_t hip_bone_index = rig->GetHumanoidBones()[(int32_t)Animation::HumanoidBone::Hips];
		if (hip_bone_index < 0)
		{
			TRC_ERROR("Rig must have a hip bone -> humanoid rig name: {}, Function: {}", rig->GetName(), __FUNCTION__);
			return false;
		}

		int32_t left_foot_bone_index = rig->GetHumanoidBones()[(int32_t)Animation::HumanoidBone::LeftFoot];
		if (left_foot_bone_index < 0)
		{
			TRC_ERROR("Rig must have a left foot bone -> humanoid rig name: {}, Function: {}", rig->GetName(), __FUNCTION__);
			return false;
		}

		int32_t right_foot_bone_index = rig->GetHumanoidBones()[(int32_t)Animation::HumanoidBone::RightFoot];
		if (right_foot_bone_index < 0)
		{
			TRC_ERROR("Rig must have a right foot bone -> humanoid rig name: {}, Function: {}", rig->GetName(), __FUNCTION__);
			return false;
		}
		
		Animation::Bone* root_bone = skeleton->GetBone(root_motion_info.root_bone_index);
		auto& channel = clip->GetTracks()[root_bone->GetStringID()];

		auto& position_track = channel[AnimationDataType::POSITION];

		float clip_duration = clip->GetDuration();
		float dt = 1.0f / float(clip->GetSampleRate());
		
		

		for (int32_t i = 1; i < position_track.size(); i++)
		{
			AnimationFrameData& frame_data = position_track[i];
			AnimationFrameData& prev_frame_data = position_track[i - 1];

			Animation::Pose frame_pose;
			frame_pose.Init(skeleton);
			AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, frame_data.time_point, &frame_pose, true);
			Transform& root_data = frame_pose.GetRootMotionDelta();

			Animation::Pose prev_frame_pose;
			prev_frame_pose.Init(skeleton);
			AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, prev_frame_data.time_point, &prev_frame_pose, true);
			Transform& prev_root_data = prev_frame_pose.GetRootMotionDelta();

			FeatureData frame_feature;
			frame_feature.root_position = root_data.GetPosition();
			


		}


		return true;
	}

}