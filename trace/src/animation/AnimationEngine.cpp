#include "pch.h"

#include "AnimationEngine.h"
#include "core/Application.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "core/Utils.h"
#include "scene/Components.h"
#include "animation/AnimationPose.h"

#include "glm/glm.hpp"



namespace trace {
	

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

		SkeletonInstance* skeleton_instance = out_pose->GetSkeletonInstance();
		Scene* scene = skeleton_instance->GetScene();

		

		if (looping)
		{
			

		}
		else if (!looping && (from_time > clip->GetDuration() || to_time > clip->GetDuration()) )
		{
			return;
		}

		SampleClip(clip, to_time, out_pose, looping);

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Transform& root_motion_delta = out_pose->GetRootMotionDelta();
		root_motion_delta = Transform::Identity();
		Bone* bone = skeleton_instance->GetSkeleton()->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return;
		}
		Transform& root_pose = out_pose->GetLocalPose()[root_motion_info.root_bone_index];
		out_pose->SetRootMotionBone(root_motion_info.root_bone_index);
		

		if (root_motion_info.Y_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			curr_position.y = 0.0f;
			root_pose.SetPosition(curr_position);
		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 curr_position = root_pose.GetPosition();
			curr_position.x = 0.0f;
			curr_position.z = 0.0f;
			root_pose.SetPosition(curr_position);
		}

		root_motion_delta = GetRootMotionDelta(clip, skeleton_instance->GetSkeleton(), from_time, to_time, looping);
		
	}

	void AnimationEngine::SampleClip(Ref<AnimationClip> clip, float time, Animation::Pose* out_pose, bool looping)
	{
		SkeletonInstance* skeleton_instance = out_pose->GetSkeletonInstance();
		Scene* scene = skeleton_instance->GetScene();

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
			int32_t bone_index = skeleton_instance->GetSkeleton()->GetBoneIndex(channel.first);

			if (bone_index < 0)
			{
				continue;
			}
			for (auto& track : channel.second)
			{
				AnimationFrameData* curr = nullptr;
				AnimationFrameData* prev = nullptr;

				//TODO: Find a better way to find which frames to sample -----------
				int32_t i = static_cast<int32_t>(track.second.size() - 1);
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

				lerp_value = glm::min(0.0f, lerp_value);


				AnimationDataType type = track.first;
				float time_point = lerp_value;
				AnimationFrameData* a = prev;
				AnimationFrameData* b = curr;
				
				std::vector<Transform>& local_pose = out_pose->GetLocalPose();
				Transform& pose = local_pose[bone_index];
				switch (type)
				{
				case AnimationDataType::POSITION:
				{
					AnimatedOutput out = CalculateData(a, b, type, time_point);
					glm::vec3 result = *(glm::vec3*)(&out.data);

					pose.SetPosition(result);
					break;
				}
				case AnimationDataType::ROTATION:
				{
					AnimatedOutput out = CalculateData(a, b, type, time_point);
					glm::quat result = *(glm::quat*)(&out.data);

					pose.SetRotation(result);
					break;
				}
				case AnimationDataType::SCALE:
				{
					AnimatedOutput out = CalculateData(a, b, type, time_point);
					glm::vec3 result = *(glm::vec3*)(&out.data);

					pose.SetScale(result);
					break;
				}

				}
			}

			
		}


	}

	Transform AnimationEngine::GetRootMotionDelta(Ref<AnimationClip> clip, Ref<Skeleton> skeleton, float from_time, float to_time, bool looping)
	{
		Transform result;
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
		Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);
		if (!bone)
		{
			return result;
		}

		auto& channel = clip->GetTracks()[bone->GetStringID()];
		auto& position_track = channel[AnimationDataType::POSITION];
		auto& rotation_track = channel[AnimationDataType::ROTATION];
		glm::vec3& _pos = *(glm::vec3*)(position_track.back().data);
		glm::quat& _rot = *(glm::quat*)(rotation_track.back().data);

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

		if (from_time > clip->GetDuration())
		{
			position += (_pos * float(f_times));
		}
		if (to_time > clip->GetDuration())
		{
			root_position += (_pos * float(t_times));
		}

		if (root_motion_info.Y_motion)
		{
			glm::vec3 pos_delta = root_position - position;
			glm::vec3 motion_delta = result.GetPosition();
			motion_delta.y = pos_delta.y;
			result.SetPosition(motion_delta);

		}

		if (root_motion_info.XZ_motion)
		{
			glm::vec3 pos_delta = root_position - position;
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

}