#include "pch.h"

#include "AnimationEngine.h"
#include "core/Application.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "core/Utils.h"
#include "scene/Components.h"
#include "animation/AnimationPose.h"
#include "debug/Debugger.h"
#include "core/Utils.h"
#include "animation/AnimationBlend.h"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"



namespace trace {
	
	//TEMP ===============
	// Used to project vector a onto vector b
	glm::vec3 project(glm::vec3& a, glm::vec3& b)
	{
		float scalar = glm::dot(a, b) / glm::dot(b, b);
		return b * scalar;
	}

	Transform get_root_bone_transform(Transform& hip_pose)
	{
		glm::vec3 forward(0.0f, 0.0f, 1.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);

		glm::vec3 _f = hip_pose.GetRotation() * forward;
		glm::vec3 projected_forward = _f - project(_f, up);
		glm::quat look_orientation = glm::quatLookAt(-projected_forward, up);
		glm::vec3 root_position = hip_pose.GetPosition() - project(hip_pose.GetPosition(), up);

		glm::quat inv_orientation = glm::normalize(glm::inverse(look_orientation));

		//Apply new hip position and orientation
		glm::vec3 hip_position = inv_orientation * (hip_pose.GetPosition() - root_position);
		glm::quat hip_orientation = inv_orientation * hip_pose.GetRotation();

		
		return Transform(root_position, look_orientation);
	}



	bool AnimationEngine::Init()
	{
		return true;
	}

	void AnimationEngine::Shutdown()
	{
	}

	void AnimationEngine::Animate(AnimationState& state, Scene* scene, std::unordered_map<StringID, UUID>& data_map)
	{

		if (!state.IsPlaying()) return;

		Ref<AnimationClip> clip = state.GetAnimationClip();

		float elasped_animation_time = Application::get_instance()->GetClock().GetElapsedTime() - state.GetStartTime();

		if (elasped_animation_time > clip->GetDuration())
		{
			if (!state.GetLoop())
			{
				state.Stop();
				return;
			}
			else elasped_animation_time = fmod(elasped_animation_time, clip->GetDuration());
		}
		state.SetElaspedTime(elasped_animation_time);




		auto lambda = [&](StringID, UUID id, AnimationDataType type, AnimationFrameData* a, AnimationFrameData* b, float time_point)
		{
			CalculateAndSetData(a, b, scene, id, type, time_point);
		};
		FindFrame(data_map, clip, scene, elasped_animation_time,  lambda);

	}

	void AnimationEngine::Animate(Ref<AnimationClip> clip, Scene* scene, float time_point, bool loop, std::unordered_map<StringID, UUID>& data_map)
	{
		float elasped_time = time_point;

		if (loop)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (time_point > clip->GetDuration())
		{
			return;
		}

		auto lambda = [&](StringID, UUID id, AnimationDataType type, AnimationFrameData* a, AnimationFrameData* b, float time_)
		{
			CalculateAndSetData(a, b, scene, id, type, time_);
		};
		FindFrame(data_map, clip, scene, elasped_time, lambda);
	}

	void AnimationEngine::Animate(AnimationComponent* animation_component, Scene* scene, float time_point, bool loop)
	{
		if (!animation_component)
		{
			return;
		}

		if (animation_component->entities.empty())
		{
			animation_component->InitializeEntities(scene);
		}

		if (animation_component->entities.empty())
		{
			TRC_WARN("Couldn't find entites, Function: {}", __FUNCTION__);
			return;
		}

		

		Animate(animation_component->animation, scene, time_point, loop, animation_component->entities);

	}

	void AnimationEngine::SampleClipWithRootMotionDelta(Ref<AnimationClip> clip, float from_time, float to_time, Animation::Pose* out_pose, bool looping)
	{

		if (!clip->HasRootMotion())
		{
			return;
		}

		Animation::SkeletonInstance* skeleton_instance = out_pose->GetSkeletonInstance();
		Ref<Animation::Skeleton> skeleton = skeleton_instance->GetSkeleton();

		SampleClipWithRootMotionDelta(clip, skeleton, from_time, to_time, out_pose, looping);
		
	}

	void AnimationEngine::SampleClipWithRootMotionDelta(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip)
		{
			return;
		}

		SampleClipWithRootMotionDelta(clip.get(), skeleton, from_time, to_time, out_pose, looping);
	}

	void AnimationEngine::SampleClipWithRootMotionDelta(int32_t last_frame_index, Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip)
		{
			return;
		}

		SampleClipWithRootMotionDelta(last_frame_index, clip.get(), skeleton, from_time, to_time, out_pose, looping);
	}

	void AnimationEngine::SampleClipWithRootMotionDelta(AnimationClip* clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip)
		{
			return;
		}

		if (!clip->HasRootMotion())
		{
			return;
		}

		Transform& root_motion_delta = out_pose->GetRootMotionDelta();
		root_motion_delta = Transform::Identity();
		if (looping)
		{


		}
		else if (!looping && (from_time > clip->GetDuration() || to_time > clip->GetDuration()))
		{
			return;
		}

		SampleClip(clip, skeleton, to_time, out_pose, looping);

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();

		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return;
		}
		Transform bind_pose(bone->GetBindPose());
		Transform& root_pose = out_pose->GetLocalPose()[root_motion_info.root_bone_index];
		out_pose->SetRootMotionBone(root_motion_info.root_bone_index);

		//TODO: Implement y motion
		if (root_motion_info.Y_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			//curr_position.y = bind_pose.GetPosition().y;
			root_pose.SetPosition(curr_position);
		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			curr_position.x = bind_pose.GetPosition().x;
			curr_position.z = bind_pose.GetPosition().z;
			root_pose.SetPosition(curr_position);
		}

		if (root_motion_info.enable_rotation)
		{
			glm::quat rotation = root_pose.GetRotation();
			glm::quat yaw_rotation = extract_yaw(rotation);
			rotation = glm::inverse(yaw_rotation) * rotation;

			root_pose.SetRotation(rotation);
		}

		root_motion_delta = GetRootMotionDelta(clip, skeleton, from_time, to_time, looping);
	}

	void AnimationEngine::SampleClipWithRootMotionDelta(int32_t last_frame_index, AnimationClip* clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip)
		{
			return;
		}

		if (!clip->HasRootMotion())
		{
			return;
		}

		Transform& root_motion_delta = out_pose->GetRootMotionDelta();
		root_motion_delta = Transform::Identity();
		if (looping)
		{


		}
		else if (!looping && (from_time > clip->GetDuration() || to_time > clip->GetDuration()))
		{
			return;
		}

		SampleClip(last_frame_index, clip, skeleton, to_time, out_pose, looping);

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();

		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return;
		}
		Transform bind_pose(bone->GetBindPose());
		Transform& root_pose = out_pose->GetLocalPose()[root_motion_info.root_bone_index];
		out_pose->SetRootMotionBone(root_motion_info.root_bone_index);

		//TODO: Implement y motion
		if (root_motion_info.Y_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			//curr_position.y = bind_pose.GetPosition().y;
			root_pose.SetPosition(curr_position);
		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			curr_position.x = bind_pose.GetPosition().x;
			curr_position.z = bind_pose.GetPosition().z;
			root_pose.SetPosition(curr_position);
		}

		if (root_motion_info.enable_rotation)
		{
			glm::quat rotation = root_pose.GetRotation();
			glm::quat yaw_rotation = extract_yaw(rotation);
			rotation = glm::inverse(yaw_rotation) * rotation;

			root_pose.SetRotation(rotation);
		}

		root_motion_delta = GetRootMotionDelta(last_frame_index, clip, skeleton, from_time, to_time, looping);
	}

	void AnimationEngine::SampleClipWithRootMotion(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, float time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip->HasRootMotion())
		{
			return;
		}

		Transform& root_motion_delta = out_pose->GetRootMotionDelta();
		root_motion_delta = Transform::Identity();
		if (looping)
		{

		}
		else if (time > clip->GetDuration())
		{
			return;
		}

		SampleClip(clip, skeleton, time, out_pose, looping);

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();

		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return;
		}
		Transform bind_pose(bone->GetBindPose());
		Transform& root_pose = out_pose->GetLocalPose()[root_motion_info.root_bone_index];
		out_pose->SetRootMotionBone(root_motion_info.root_bone_index);

		//TODO: Implement y motion
		if (root_motion_info.Y_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			//curr_position.y = bind_pose.GetPosition().y;
			root_pose.SetPosition(curr_position);
		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			glm::vec3 root_position = root_motion_delta.GetPosition();
			root_position.x = curr_position.x;
			root_position.z = curr_position.z;
			curr_position.x = 0.0f;
			curr_position.z = 0.0f;

			root_pose.SetPosition(curr_position);
			root_motion_delta.SetPosition(root_position);
		}

		if (root_motion_info.enable_rotation)
		{
			glm::quat rotation = root_pose.GetRotation();
			glm::quat yaw_rotation = extract_yaw(rotation);
			rotation = glm::inverse(yaw_rotation) * rotation;

			root_pose.SetRotation(rotation);
			root_motion_delta.SetRotation(yaw_rotation);
		}

	}

	void AnimationEngine::SampleClip(Ref<AnimationClip> clip, float time, Animation::Pose* out_pose, bool looping)
	{
		Animation::SkeletonInstance* skeleton_instance = out_pose->GetSkeletonInstance();
		
		SampleClip(clip, skeleton_instance->GetSkeleton(), time, out_pose, looping);

	}

	void AnimationEngine::SampleClip(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, float time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip)
		{
			return;
		}
		SampleClip(clip.get(), skeleton, time, out_pose, looping);
	}

	void AnimationEngine::SampleClip(int32_t last_frame_index, Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, float time, Animation::Pose* out_pose, bool looping)
	{
		if (!clip)
		{
			return;
		}
		SampleClip(last_frame_index, clip.get(), skeleton, time, out_pose, looping);
	}

	void AnimationEngine::SampleClip(AnimationClip* clip, Ref<Animation::Skeleton> skeleton, float time, Animation::Pose* out_pose, bool looping)
	{
		float elasped_time = time;

		if (looping)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (!looping && time > clip->GetDuration())
		{
			return;
		}

		for (auto& channel : clip->GetTracks())
		{
			int32_t bone_index = skeleton->GetBoneIndex(channel.first);

			if (bone_index < 0)
			{
				continue;
			}

			std::vector<Transform>& local_pose = out_pose->GetLocalPose();
			Transform& pose = local_pose[bone_index];

			if (channel.second.find(AnimationDataType::POSITION) != channel.second.end())
			{
				AnimatedOutput pos_out = GetFrameData(clip, channel.second, AnimationDataType::POSITION, time, looping);
				glm::vec3 pos_result = *(glm::vec3*)(&pos_out.data);

				pose.SetPosition(pos_result);
			}

			if (channel.second.find(AnimationDataType::ROTATION) != channel.second.end())
			{
				AnimatedOutput rot_out = GetFrameData(clip, channel.second, AnimationDataType::ROTATION, time, looping);
				glm::quat rot_result = *(glm::quat*)(&rot_out.data);

				pose.SetRotation(rot_result);
			}

			if (channel.second.find(AnimationDataType::SCALE) != channel.second.end())
			{
				AnimatedOutput scale_out = GetFrameData(clip, channel.second, AnimationDataType::SCALE, time, looping);
				glm::vec3 scale_result = *(glm::vec3*)(&scale_out.data);

				pose.SetScale(scale_result);
			}
		}
	}

	void AnimationEngine::SampleClip(int32_t last_frame_index, AnimationClip* clip, Ref<Animation::Skeleton> skeleton, float time, Animation::Pose* out_pose, bool looping)
	{
		float elasped_time = time;

		if (looping)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (!looping && time > clip->GetDuration())
		{
			return;
		}

		for (auto& channel : clip->GetTracks())
		{
			int32_t bone_index = skeleton->GetBoneIndex(channel.first);

			if (bone_index < 0)
			{
				continue;
			}

			std::vector<Transform>& local_pose = out_pose->GetLocalPose();
			Transform& pose = local_pose[bone_index];

			if (channel.second.find(AnimationDataType::POSITION) != channel.second.end())
			{
				AnimatedOutput pos_out = GetFrameData(last_frame_index, clip, channel.second, AnimationDataType::POSITION, time, looping);
				glm::vec3 pos_result = *(glm::vec3*)(&pos_out.data);

				pose.SetPosition(pos_result);
			}

			if (channel.second.find(AnimationDataType::ROTATION) != channel.second.end())
			{
				AnimatedOutput rot_out = GetFrameData(last_frame_index, clip, channel.second, AnimationDataType::ROTATION, time, looping);
				glm::quat rot_result = *(glm::quat*)(&rot_out.data);

				pose.SetRotation(rot_result);
			}

			if (channel.second.find(AnimationDataType::SCALE) != channel.second.end())
			{
				AnimatedOutput scale_out = GetFrameData(last_frame_index, clip, channel.second, AnimationDataType::SCALE, time, looping);
				glm::vec3 scale_result = *(glm::vec3*)(&scale_out.data);

				pose.SetScale(scale_result);
			}
		}
	}

	

	Transform AnimationEngine::GetRootMotionDelta(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, bool looping)
	{
		Transform result;
		if (!clip)
		{
			return result;
		}

		result = GetRootMotionDelta(clip.get(), skeleton, from_time, to_time, looping);
		return result;
	}

	Transform AnimationEngine::GetRootMotionDelta(AnimationClip* clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, bool looping)
	{
		Transform result;

		if (!clip)
		{
			return result;
		}

		if (!clip->HasRootMotion())
		{
			return result;
		}


		if (looping)
		{

		}
		else if (!looping && (from_time > clip->GetDuration() || to_time > clip->GetDuration()))
		{
			return result;
		}

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return result;
		}

		float from_elasped_time = fmod(from_time, clip->GetDuration());
		float to_elasped_time = fmod(to_time, clip->GetDuration());

		auto& channel = clip->GetTracks()[bone->GetStringID()];
		auto& position_track = channel[AnimationDataType::POSITION];
		auto& rotation_track = channel[AnimationDataType::ROTATION];
		glm::vec3& last_pos = *(glm::vec3*)(position_track.back().data);
		glm::quat& last_rot = *(glm::quat*)(rotation_track.back().data);

		glm::vec3& first_pos = *(glm::vec3*)(position_track.front().data);
		glm::quat& first_rot = *(glm::quat*)(rotation_track.front().data);

		glm::vec3 diff_pos = last_pos - first_pos;
		glm::quat diff_rot = last_rot * glm::inverse(first_rot);
		diff_rot = glm::normalize(diff_rot);



		AnimatedOutput pos_output = GetFrameData(clip, channel, AnimationDataType::POSITION, from_time, looping);
		AnimatedOutput rot_output = GetFrameData(clip, channel, AnimationDataType::ROTATION, from_time, looping);
		glm::vec3 position = *(glm::vec3*)(&pos_output.data);
		glm::quat rotation = *(glm::quat*)(&rot_output.data);



		AnimatedOutput root_pos_output = GetFrameData(clip, channel, AnimationDataType::POSITION, to_time, looping);
		AnimatedOutput root_rot_output = GetFrameData(clip, channel, AnimationDataType::ROTATION, to_time, looping);
		glm::vec3 root_position = *(glm::vec3*)(&root_pos_output.data);
		glm::quat root_rotation = *(glm::quat*)(&root_rot_output.data);


		int f_times = int(from_time / clip->GetDuration());
		int t_times = int(to_time / clip->GetDuration());


		if (from_elasped_time > to_elasped_time)
		{
			root_position += diff_pos;
			root_rotation = glm::normalize(root_rotation * diff_rot);
		}

		glm::quat rot_delta = extract_yaw(root_rotation) * glm::inverse(extract_yaw(rotation));
		rot_delta = glm::normalize(rot_delta);
		glm::vec3 pos_delta = root_position - position;

		if (root_motion_info.enable_rotation)
		{

			rot_delta = (rot_delta);
			result.SetRotation(rot_delta);

			glm::quat inv_rot = glm::inverse(extract_yaw(rotation));
			pos_delta = inv_rot * pos_delta;



		}

		//TODO: Implement y motion
		if (root_motion_info.Y_motion)
		{
			glm::vec3 motion_delta = result.GetPosition();
			//motion_delta.y = pos_delta.y;
			result.SetPosition(motion_delta);

		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 motion_delta = result.GetPosition();
			motion_delta.x = pos_delta.x;
			motion_delta.z = pos_delta.z;
			result.SetPosition(motion_delta);
		}



		return result;
	}

	Transform AnimationEngine::GetRootMotionDelta(int32_t last_frame_index, AnimationClip* clip, Ref<Animation::Skeleton> skeleton, float from_time, float to_time, bool looping)
	{
		Transform result;

		if (!clip)
		{
			return result;
		}

		if (!clip->HasRootMotion())
		{
			return result;
		}


		if (looping)
		{

		}
		else if (!looping && (from_time > clip->GetDuration() || to_time > clip->GetDuration()))
		{
			return result;
		}

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return result;
		}

		float from_elasped_time = fmod(from_time, clip->GetDuration());
		float to_elasped_time = fmod(to_time, clip->GetDuration());

		auto& channel = clip->GetTracks()[bone->GetStringID()];
		auto& position_track = channel[AnimationDataType::POSITION];
		auto& rotation_track = channel[AnimationDataType::ROTATION];
		glm::vec3& last_pos = *(glm::vec3*)(position_track.back().data);
		glm::quat& last_rot = *(glm::quat*)(rotation_track.back().data);

		glm::vec3& first_pos = *(glm::vec3*)(position_track.front().data);
		glm::quat& first_rot = *(glm::quat*)(rotation_track.front().data);

		glm::vec3 diff_pos = last_pos - first_pos;
		glm::quat diff_rot = last_rot * glm::inverse(first_rot);
		diff_rot = glm::normalize(diff_rot);



		AnimatedOutput pos_output = GetFrameData(last_frame_index, clip, channel, AnimationDataType::POSITION, from_time, looping);
		AnimatedOutput rot_output = GetFrameData(last_frame_index, clip, channel, AnimationDataType::ROTATION, from_time, looping);
		glm::vec3 position = *(glm::vec3*)(&pos_output.data);
		glm::quat rotation = *(glm::quat*)(&rot_output.data);



		AnimatedOutput root_pos_output = GetFrameData(last_frame_index, clip, channel, AnimationDataType::POSITION, to_time, looping);
		AnimatedOutput root_rot_output = GetFrameData(last_frame_index, clip, channel, AnimationDataType::ROTATION, to_time, looping);
		glm::vec3 root_position = *(glm::vec3*)(&root_pos_output.data);
		glm::quat root_rotation = *(glm::quat*)(&root_rot_output.data);


		int f_times = int(from_time / clip->GetDuration());
		int t_times = int(to_time / clip->GetDuration());


		if (from_elasped_time > to_elasped_time)
		{
			root_position += diff_pos;
			root_rotation = glm::normalize(root_rotation * diff_rot);
		}

		glm::quat rot_delta = extract_yaw(root_rotation) * glm::inverse(extract_yaw(rotation));
		rot_delta = glm::normalize(rot_delta);
		glm::vec3 pos_delta = root_position - position;

		if (root_motion_info.enable_rotation)
		{

			rot_delta = (rot_delta);
			result.SetRotation(rot_delta);

			glm::quat inv_rot = glm::inverse(extract_yaw(rotation));
			pos_delta = inv_rot * pos_delta;



		}

		//TODO: Implement y motion
		if (root_motion_info.Y_motion)
		{
			glm::vec3 motion_delta = result.GetPosition();
			//motion_delta.y = pos_delta.y;
			result.SetPosition(motion_delta);

		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 motion_delta = result.GetPosition();
			motion_delta.x = pos_delta.x;
			motion_delta.z = pos_delta.z;
			result.SetPosition(motion_delta);
		}



		return result;
	}

	Transform AnimationEngine::GetRootMotionDelta(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton, std::vector<Transform>& root_motion_data, float from_time, float to_time, bool looping)
	{
		Transform result = Transform::Identity();
		if (looping)
		{

		}
		else if (!looping && (from_time > clip->GetDuration() || to_time > clip->GetDuration()))
		{
			return result;
		}

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return result;
		}

		float from_elasped_time = fmod(from_time, clip->GetDuration());
		float to_elasped_time = fmod(to_time, clip->GetDuration());

		auto& channel = clip->GetTracks()[bone->GetStringID()];

		auto find_value = [&](float time) -> Transform 
		{
			int32_t num_frames = root_motion_data.size();
			float lerp_value = 0.0f;
			int32_t index = GetFrameIndex(clip.get(), channel, AnimationDataType::POSITION, time, looping, &lerp_value);

			Transform& _a = root_motion_data[index];

			if (index < (num_frames - 1))
			{
				Transform& _b = root_motion_data[index + 1];
				Transform result;
				Animation::BlendTransform(&_a, &_b, &result, lerp_value);
				return result;
			}

			return _a;
		};

		glm::vec3 last_pos = root_motion_data.back().GetPosition();
		glm::quat last_rot = root_motion_data.back().GetRotation();

		glm::vec3 first_pos = root_motion_data.front().GetPosition();
		glm::quat first_rot = root_motion_data.front().GetRotation();

		glm::vec3 diff_pos = last_pos - first_pos;
		glm::quat diff_rot = last_rot * glm::inverse(first_rot);
		diff_rot = glm::normalize(diff_rot);



		Transform prev_pose = find_value(from_time);
		glm::vec3 position = prev_pose.GetPosition();
		glm::quat rotation = prev_pose.GetRotation();



		Transform curr_pose = find_value(to_time);
		glm::vec3 root_position = curr_pose.GetPosition();
		glm::quat root_rotation = curr_pose.GetRotation();

		if (from_elasped_time > to_elasped_time)
		{
			root_position += diff_pos;
			root_rotation = glm::normalize(root_rotation * diff_rot);
		}

		glm::quat rot_delta = extract_yaw(root_rotation) * glm::inverse(extract_yaw(rotation));
		rot_delta = glm::normalize(rot_delta);
		glm::vec3 pos_delta = root_position - position;

		if (root_motion_info.enable_rotation)
		{

			rot_delta = (rot_delta);
			result.SetRotation(rot_delta);

			glm::quat inv_rot = glm::normalize(glm::inverse(extract_yaw(rotation)));
			pos_delta = inv_rot * pos_delta;

			

		}

		if (root_motion_info.Y_motion)
		{
			glm::vec3 motion_delta = result.GetPosition();
			motion_delta.y = pos_delta.y;
			result.SetPosition(motion_delta);

		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 motion_delta = result.GetPosition();
			motion_delta.x = pos_delta.x;
			motion_delta.z = pos_delta.z;
			result.SetPosition(motion_delta);
		}



		return result;
	}

	AnimationEngine* AnimationEngine::get_instance()
	{
		static AnimationEngine* s_instance = new AnimationEngine();
		return s_instance;
	}

	void AnimationEngine::CalculateAndSetData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, Scene* scene, UUID id, AnimationDataType type, float time_point)
	{
		SetData(CalculateData(frame_a, frame_b, type, time_point), scene, id);
	}

	AnimatedOutput AnimationEngine::CalculateData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, AnimationDataType type, float time_point)
	{
		AnimatedOutput output;
		output.type = type;

		switch (type)
		{
		case AnimationDataType::NONE:
		{
			break;
		}
		case AnimationDataType::POSITION:
		{
			glm::vec3& _a = *(glm::vec3*)(&frame_a->data);
			glm::vec3& _b = *(glm::vec3*)(&frame_b->data);

			glm::vec3 result(0.0f);
			result.x = lerp(_a.x, _b.x, time_point);
			result.y = lerp(_a.y, _b.y, time_point);
			result.z = lerp(_a.z, _b.z, time_point);

			memcpy(output.data, &result, sizeof(glm::vec3));
			break;
		}
		case AnimationDataType::ROTATION:
		{
			glm::quat& _a = *(glm::quat*)(&frame_a->data);
			glm::quat& _b = *(glm::quat*)(&frame_b->data);

			glm::quat result;
			result = glm::slerp(_a, _b, time_point);

			memcpy(output.data, &result, sizeof(glm::quat));
			break;
		}
		case AnimationDataType::SCALE:
		{
			glm::vec3& _a = *(glm::vec3*)(&frame_a->data);
			glm::vec3& _b = *(glm::vec3*)(&frame_b->data);

			glm::vec3 result(0.0f);
			result.x = lerp(_a.x, _b.x, time_point);
			result.y = lerp(_a.y, _b.y, time_point);
			result.z = lerp(_a.z, _b.z, time_point);

			memcpy(output.data, &result, sizeof(glm::vec3));

			break;
		}
		case AnimationDataType::TEXT_INTENSITY:
		{
			float& _a = *(float*)(&frame_a->data);
			float& _b = *(float*)(&frame_b->data);

			float result(0.0f);
			result = lerp(_a, _b, time_point);

			memcpy(output.data, &result, sizeof(float));
			break;
		}
		case AnimationDataType::LIGHT_INTENSITY:
		{
			float& _a = *(float*)(&frame_a->data);
			float& _b = *(float*)(&frame_b->data);

			float result(0.0f);
			result = lerp(_a, _b, time_point);

			memcpy(output.data, &result, sizeof(float));
			break;
		}
		case AnimationDataType::IMAGE_COLOR:
		{
			uint32_t& _a = *(uint32_t*)(&frame_a->data);
			uint32_t& _b = *(uint32_t*)(&frame_b->data);

			uint32_t result = TRC_COL32_WHITE;

			glm::vec4 color_a = colorFromUint32(_a);
			glm::vec4 color_b = colorFromUint32(_b);

			glm::vec4 color_result(0.0f);
			color_result.r = lerp(color_a.r, color_b.r, time_point);
			color_result.g = lerp(color_a.g, color_b.g, time_point);
			color_result.b = lerp(color_a.b, color_b.b, time_point);
			color_result.a = lerp(color_a.a, color_b.a, time_point);

			result = colorVec4ToUint(color_result);

			memcpy(output.data, &result, sizeof(uint32_t));
			break;
		}
		}

		return output;
	}

	void AnimationEngine::SetData(AnimatedOutput& data, Scene* scene, UUID id)
	{
		Entity entity = scene->GetEntity(id);

		if (!entity)
		{
			TRC_ERROR("Invalid entity: File->{}, Line->{}", __FILE__, __LINE__);
			return;
		}

		switch (data.type)
		{
		case AnimationDataType::NONE:
		{
			break;
		}
		case AnimationDataType::POSITION:
		{

			glm::vec3 result = *(glm::vec3*)(&data.data);

			entity.GetComponent<TransformComponent>()._transform.SetPosition(result);
			break;
		}
		case AnimationDataType::ROTATION:
		{
			glm::quat result = *(glm::quat*)(&data.data);


			entity.GetComponent<TransformComponent>()._transform.SetRotation(result);
			break;
		}
		case AnimationDataType::SCALE:
		{
			glm::vec3 result = *(glm::vec3*)(&data.data);

			entity.GetComponent<TransformComponent>()._transform.SetScale(result);

			break;
		}
		case AnimationDataType::TEXT_INTENSITY:
		{
			float result = *(float*)(&data.data);


			entity.GetComponent<TextComponent>().intensity = result;
			break;
		}
		case AnimationDataType::LIGHT_INTENSITY:
		{
			float result = *(float*)(&data.data);


			entity.GetComponent<LightComponent>()._light.params2.y = result;
			break;
		}
		case AnimationDataType::IMAGE_COLOR:
		{
			uint32_t result = *(uint32_t*)(&data.data);


			entity.GetComponent<ImageComponent>().color = result;
			break;
		}
		}

	}

	void AnimationEngine::FindFrame(std::unordered_map<StringID, UUID>& data_map, Ref<AnimationClip> clip, Scene* scene, float elasped_time, std::function<void(StringID, UUID, AnimationDataType , AnimationFrameData* , AnimationFrameData* , float )> callback)
	{

		for (auto& channel : clip->GetTracks())
		{
			auto it = data_map.find(channel.first);
			if (it == data_map.end())
			{
				continue;
			}
			UUID object = it->second;
			for (auto& track : channel.second)
			{
				AnimationFrameData* curr = nullptr;
				AnimationFrameData* prev = nullptr;

				//TODO: Find a better way to find which frames to sample -----------
				int32_t i = static_cast<int32_t>(track.second.size() - 1);
				if (track.second[i].time_point < elasped_time)
				{
					curr = &track.second[i];
				}
				if (!curr)
				{
					for (; i >= 0; i--)
					{
						curr = &track.second[i];
						prev = i != 0 ? &track.second[i - 1] : nullptr;
						if (elasped_time <= curr->time_point)
						{
							if (prev && elasped_time >= prev->time_point)
							{
								break;
							}
						}


					}
				}
				
				// -----------------------------------------------------------------

				if (!prev && !curr)
				{
					continue;
				}

				if (prev && !curr)
				{
					curr = prev;
				}

				if (!prev && curr)
				{
					prev = curr;
				}


				float lerp_value = (elasped_time - prev->time_point) / (curr->time_point - prev->time_point);

				lerp_value = std::clamp(lerp_value, 0.0f, 1.0f);


				callback(channel.first, object, track.first, prev, curr, lerp_value);
			}
		}


	}

	AnimatedOutput AnimationEngine::GetFrameData(Ref<AnimationClip> clip, AnimationDataTrack& channel , AnimationDataType type, float time, bool looping)
	{
		return GetFrameData(clip.get(), channel, type, time, looping);
	}

	AnimatedOutput AnimationEngine::GetFrameData(AnimationClip* clip, AnimationDataTrack& channel, AnimationDataType type, float time, bool looping)
	{
		float elasped_time = time;

		if (looping)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (!looping && time > clip->GetDuration())
		{
			return AnimatedOutput();
		}

		auto& track = channel[type];

		AnimationFrameData* curr = nullptr;
		AnimationFrameData* prev = nullptr;

		//TODO: Find a better way to find which frames to sample -----------
		int32_t i = static_cast<int32_t>(track.size() - 1);
		for (; i >= 0; i--)
		{
			curr = &track[i];
			prev = i != 0 ? &track[i - 1] : nullptr;
			if (elasped_time <= curr->time_point)
			{
				if (prev && elasped_time >= prev->time_point)
				{
					break;
				}
			}

		}
		// -----------------------------------------------------------------

		if (!prev && !curr)
		{
			return AnimatedOutput();
		}

		if (prev && !curr)
		{
			curr = prev;
		}

		if (!prev && curr)
		{
			prev = curr;
		}


		float lerp_value = (elasped_time - prev->time_point) / (curr->time_point - prev->time_point);

		lerp_value = glm::min(0.0f, lerp_value);


		float time_point = lerp_value;
		AnimationFrameData* a = prev;
		AnimationFrameData* b = curr;

		return CalculateData(a, b, type, time_point);
	}

	AnimatedOutput AnimationEngine::GetFrameData(int32_t last_frame_index, AnimationClip* clip, AnimationDataTrack& channel, AnimationDataType type, float time, bool looping)
	{
		auto& track = channel[type];
		if (last_frame_index < 0 || last_frame_index > (track.size() - 1))
		{
			return GetFrameData(clip, channel, type, time, looping);
		}

		float elasped_time = time;

		if (looping)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (!looping && time > clip->GetDuration())
		{
			return AnimatedOutput();
		}


		AnimationFrameData* last_frame = &track[last_frame_index];
		float last_frame_time = last_frame->time_point;
		AnimationFrameData* curr = nullptr;
		AnimationFrameData* prev = nullptr;

		//TODO: Find a better way to find which frames to sample -----------
		int32_t i = static_cast<int32_t>(track.size() - 1);
		if (last_frame_time < elasped_time)
		{
			i = last_frame_index;
			for (; i < track.size() - 1; i++)
			{
				curr = &track[i];
				prev = i != 0 ? &track[i - 1] : nullptr;
				if (elasped_time <= curr->time_point)
				{
					if (prev && elasped_time >= prev->time_point)
					{
						break;
					}
				}

			}
		}
		else if (last_frame_time >= elasped_time)
		{
			i = last_frame_index + 1;
			for (; i >= 0; i--)
			{
				curr = &track[i];
				prev = i != 0 ? &track[i - 1] : nullptr;
				if (elasped_time <= curr->time_point)
				{
					if (prev && elasped_time >= prev->time_point)
					{
						break;
					}
				}

			}
		}
		else
		{
			TRC_ASSERT(false, "This block of code should not be reached, Function: {}", __FUNCTION__);
		}
		// -----------------------------------------------------------------

		if (!prev && !curr)
		{
			return AnimatedOutput();
		}

		if (prev && !curr)
		{
			curr = prev;
		}

		if (!prev && curr)
		{
			prev = curr;
		}


		float lerp_value = (elasped_time - prev->time_point) / (curr->time_point - prev->time_point);

		lerp_value = glm::min(0.0f, lerp_value);


		float time_point = lerp_value;
		AnimationFrameData* a = prev;
		AnimationFrameData* b = curr;

		return CalculateData(a, b, type, time_point);
	}

	int32_t AnimationEngine::GetFrameIndex(AnimationClip* clip, AnimationDataTrack& channel, AnimationDataType type, float time, bool looping, float* out_lerp_value)
	{
		float elasped_time = time;

		if (looping)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (!looping && time > clip->GetDuration())
		{
			return -1;
		}

		auto& track = channel[type];

		AnimationFrameData* curr = nullptr;
		AnimationFrameData* prev = nullptr;

		//TODO: Find a better way to find which frames to sample -----------
		int32_t i = static_cast<int32_t>(track.size() - 1);
		for (; i >= 0; i--)
		{
			curr = &track[i];
			prev = i != 0 ? &track[i - 1] : nullptr;
			if (elasped_time <= curr->time_point)
			{
				if (prev && elasped_time >= prev->time_point)
				{
					break;
				}
			}

		}
		// -----------------------------------------------------------------
		int32_t result = i - 1;
		if (!prev && !curr)
		{
			return 0;
		}

		if (!prev && curr)
		{
			result = 0;
		}


		float lerp_value = (elasped_time - prev->time_point) / (curr->time_point - prev->time_point);

		lerp_value = glm::min(0.0f, lerp_value);

		if (out_lerp_value)
		{
			*out_lerp_value = lerp_value;
		}

		return result;
	}

	int32_t AnimationEngine::GetFrameIndex(int32_t last_frame_index, AnimationClip* clip, AnimationDataTrack& channel, AnimationDataType type, float time, bool looping, float* out_lerp_value)
	{
		auto& track = channel[type];
		if (last_frame_index < 0 || last_frame_index >(track.size() - 1))
		{
			return GetFrameIndex(clip, channel, type, time, looping, out_lerp_value);
		}

		float elasped_time = time;

		if (looping)
		{
			elasped_time = fmod(elasped_time, clip->GetDuration());
		}
		else if (!looping && time > clip->GetDuration())
		{
			return -1;
		}


		AnimationFrameData* last_frame = &track[last_frame_index];
		float last_frame_time = last_frame->time_point;
		AnimationFrameData* curr = nullptr;
		AnimationFrameData* prev = nullptr;

		//TODO: Find a better way to find which frames to sample -----------
		int32_t i = static_cast<int32_t>(track.size() - 1);
		if (last_frame_time < elasped_time)
		{
			i = last_frame_index;
			for (; i < track.size() - 1; i++)
			{
				curr = &track[i];
				prev = i != 0 ? &track[i - 1] : nullptr;
				if (elasped_time <= curr->time_point)
				{
					if (prev && elasped_time >= prev->time_point)
					{
						break;
					}
				}

			}
		}
		else if (last_frame_time >= elasped_time)
		{
			i = last_frame_index + 1;
			for (; i >= 0; i--)
			{
				curr = &track[i];
				prev = i != 0 ? &track[i - 1] : nullptr;
				if (elasped_time <= curr->time_point)
				{
					if (prev && elasped_time >= prev->time_point)
					{
						break;
					}
				}

			}
		}
		else
		{
			TRC_ASSERT(false, "This block of code should not be reached, Function: {}", __FUNCTION__);
		}
		// -----------------------------------------------------------------
		int32_t result = i - 1;
		if (!prev && !curr)
		{
			return 0;
		}

		if (!prev && curr)
		{
			result = 0;
		}


		float lerp_value = (elasped_time - prev->time_point) / (curr->time_point - prev->time_point);

		lerp_value = glm::min(0.0f, lerp_value);

		if (out_lerp_value)
		{
			*out_lerp_value = lerp_value;
		}

		return result;
	}

}