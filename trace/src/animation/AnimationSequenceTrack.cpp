#include "pch.h"

#include "animation/SequenceTrackChannel.h"
#include "animation/AnimationSequenceTrack.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "animation/AnimationSequence.h"
#include "core/io/Logging.h"
#include "animation/AnimationEngine.h"
#include "animation/AnimationBlend.h"
#include "animation/AnimationPose.h"
#include "core/Utils.h"

namespace trace::Animation {

	bool AnimationSequenceTrack::Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{

		m_type = SequenceTrackType::ANIMATION_TRACK;

		std::vector<void*>& tracks_data = instance->GetTracksData();

		RuntimeData* data = new RuntimeData; //TODO: Use custom allocator
		tracks_data[track_index] = data;

		data->current_channel_index = 0;

		Entity entity = scene->GetEntityByName(m_stringID);

		if (!entity)
		{
			TRC_ERROR("Invalid Entity, Function: {}", __FUNCTION__);
			return false;
		}

		data->entity_id = entity.GetID();

		for (uint32_t i = 0; i < m_channels.size(); i++)
		{
			AnimationChannel* channel = (AnimationChannel*)m_channels[i];

			Ref<AnimationClip> clip = channel->GetAnimationClip();

			if (!clip)
			{
				continue;
			}

			for (auto& object : clip->GetTracks())
			{
				Entity obj = scene->GetEntityByName(object.first);
				if (!obj)
				{
					continue;
				}

				data->m_runtime_entites[object.first] = obj.GetID();
			}
		}


		return true;
	}

	AnimationSequenceTrack::AnimationSequenceTrack()
	{
		m_type = SequenceTrackType::ANIMATION_TRACK;
	}

	void AnimationSequenceTrack::Update(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{
		std::vector<void*>& tracks_data = instance->GetTracksData();

		if (m_channels.empty())
		{
			return;
		}

		RuntimeData* data = (RuntimeData*)tracks_data[track_index];

		if (data->entity_id == 0)
		{
			return;
		}

		if (!IsTimeWithinChannel(m_channels[data->current_channel_index], instance->GetElaspedTime()))
		{
			int32_t index = FindChannelWithinTime(instance->GetElaspedTime(), data->current_channel_index);
			if (index < 0)
			{
				return;
			}
			data->current_channel_index = index;
		}

		AnimationChannel* channel = (AnimationChannel*)m_channels[data->current_channel_index];
		Ref<AnimationClip> clip = channel->GetAnimationClip();
		if (!clip)
		{
			return;
		}
		float sample_time = instance->GetElaspedTime() - channel->GetStartTime();
		AnimationEngine::get_instance()->Animate(clip, scene, sample_time, true, data->m_runtime_entites);

	}

	bool SkeletalAnimationTrack::Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{
		m_type = SequenceTrackType::SKELETAL_ANIMATION_TRACK;

		std::vector<void*>& tracks_data = instance->GetTracksData();

		RuntimeData* data = new RuntimeData; //TODO: Use custom allocator
		tracks_data[track_index] = data;

		data->current_channel_index = 0;

		Entity entity = scene->GetEntityByName(m_stringID);

		if (!entity)
		{
			TRC_ERROR("Invalid Entity, Function: {}", __FUNCTION__);
			return false;
		}

		if (!entity.HasComponent<AnimationGraphController>())
		{
			TRC_ERROR("Invalid Entity with no animation graph component, Name: {} Function: {}", entity.GetComponent<TagComponent>().GetTag(), __FUNCTION__);
			return false;
		}

		data->entity_id = entity.GetID();

		Transform& pose = entity.GetComponent<TransformComponent>()._transform;
		data->start_entity_position = pose.GetPosition();
		data->start_entity_rotation = pose.GetRotation();

		AnimationGraphController& controller = entity.GetComponent<AnimationGraphController>();
		GraphInstance& graph = controller.graph;
		data->skeletal_instance.CreateInstance(graph.GetGraph()->GetSkeleton(), scene, data->entity_id);
		data->skeleton_pose.Init(&data->skeletal_instance);
		data->blend_pose.Init(&data->skeletal_instance);


		return true;
	}

	SkeletalAnimationTrack::SkeletalAnimationTrack()
	{
		m_type = SequenceTrackType::SKELETAL_ANIMATION_TRACK;
	}

	void SkeletalAnimationTrack::Update(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{

		std::vector<void*>& tracks_data = instance->GetTracksData();

		if (m_channels.empty())
		{
			return;
		}

		RuntimeData* data = (RuntimeData*)tracks_data[track_index];


		if (data->skeletal_instance.GetEntites().empty())
		{
			return;
		}
		

		if (!IsTimeWithinChannel(m_channels[data->current_channel_index], instance->GetElaspedTime()))
		{
			int32_t index = FindChannelWithinTime(instance->GetElaspedTime(), data->current_channel_index);
			if (index < 0)
			{
				return;
			}
			data->current_channel_index = index;
		}

		SkeletalAnimationChannel* channel = (SkeletalAnimationChannel*)m_channels[data->current_channel_index];
		Ref<AnimationClip> clip = channel->GetAnimationClip();
		if (!clip)
		{
			return;
		}
		float sample_time = instance->GetElaspedTime() - channel->GetStartTime();

		/*AnimationEngine::get_instance()->SampleClip(clip, sample_time, &data->skeleton_pose, true);

		int32_t next_index = data->current_channel_index + 1;
		if ( next_index < m_channels.size() && IsTimeWithinChannel(m_channels[next_index], instance->GetElaspedTime()))
		{
			
			SkeletalAnimationChannel* next_channel = (SkeletalAnimationChannel*)m_channels[next_index];
			float next_sample_time = instance->GetElaspedTime() - m_channels[next_index]->GetStartTime();

			float channel_end_time = channel->GetStartTime() + channel->GetDuration();
			float blend_duration = channel_end_time - next_channel->GetStartTime();
			float blend_weight = (instance->GetElaspedTime() - next_channel->GetStartTime()) / blend_duration;

			Ref<AnimationClip> next_clip = next_channel->GetAnimationClip();

			if (next_clip)
			{
				AnimationEngine::get_instance()->SampleClip(next_clip, next_sample_time, &data->blend_pose, true);
				BlendPose(&data->skeleton_pose, &data->blend_pose, &data->skeleton_pose, blend_weight);
			}
		}

		int32_t prev_index = data->current_channel_index - 1;
		if (prev_index >= 0 && IsTimeWithinChannel(m_channels[prev_index], instance->GetElaspedTime()))
		{

			SkeletalAnimationChannel* prev_channel = (SkeletalAnimationChannel*)m_channels[prev_index];
			float prev_sample_time = instance->GetElaspedTime() - m_channels[prev_index]->GetStartTime();
			float prev_end_time = m_channels[prev_index]->GetStartTime() + m_channels[prev_index]->GetDuration();

			float channel_end_time = channel->GetStartTime() + channel->GetDuration();
			float blend_duration = prev_end_time - channel->GetStartTime() ;
			float blend_weight = (instance->GetElaspedTime() - channel->GetStartTime()) / blend_duration;

			Ref<AnimationClip> prev_clip = prev_channel->GetAnimationClip();

			if (prev_clip)
			{
				AnimationEngine::get_instance()->SampleClip(prev_clip, prev_sample_time, &data->blend_pose, true);
				BlendPose(&data->skeleton_pose, &data->blend_pose, &data->skeleton_pose, 1.0f - blend_weight);
			}
		}

		data->skeleton_pose.SetEntityLocalPose();*/

		// ============================================================

		Transform root_motion_delta = Transform::Identity();
		glm::vec3 total_pos_delta = glm::vec3(0.0f);
		for (int32_t i = 0; i < m_channels.size(); i++)
		{
			SkeletalAnimationChannel* sk_channel = (SkeletalAnimationChannel*)m_channels[i];

			if (instance->GetElaspedTime() < sk_channel->GetStartTime())
			{
				break;
			}


			float end_time = sk_channel->GetEndTime();
			Ref<AnimationClip> clip = sk_channel->GetAnimationClip();
			if (!clip)
			{
				continue;
			}

			if (clip->HasRootMotion() && IsTimeWithinChannel(sk_channel, instance->GetElaspedTime()))
			{
				if (IsPrevWithinChannel(i))
				{
					float blend_duration = 0.0f;
					float blend_weight = 0.0f;
					GetPrevBlend(i, instance->GetElaspedTime(), blend_weight, blend_duration);
					if (sample_time > blend_duration)
					{
						break;
					}
				}
				AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(clip, 0.0f, sample_time, &data->skeleton_pose, true);

				float blend_duration = 0.0f;
				float blend_weight = 0.0f;
				if (GetNextBlend(i, instance->GetElaspedTime(), blend_weight, blend_duration))
				{
					int32_t next_index = i + 1;
					SkeletalAnimationChannel* next_channel = (SkeletalAnimationChannel*)m_channels[next_index];
					Ref<AnimationClip> next_clip = next_channel->GetAnimationClip();
					float next_sample_time = instance->GetElaspedTime() - next_channel->GetStartTime();
					if (next_clip)
					{
						AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(next_clip, 0.0f, next_sample_time, &data->blend_pose, true);
						BlendPose(&data->skeleton_pose, &data->blend_pose, &data->skeleton_pose, blend_weight);
					}
				}
				data->skeleton_pose.SetEntityLocalPose();
				total_pos_delta += data->skeleton_pose.GetRootMotionDelta().GetPosition();
				break;
			}
			else if(!clip->HasRootMotion() && IsTimeWithinChannel(sk_channel, instance->GetElaspedTime()))
			{
				if (IsPrevWithinChannel(i))
				{
					float blend_duration = 0.0f;
					float blend_weight = 0.0f;
					GetPrevBlend(i, instance->GetElaspedTime(), blend_weight, blend_duration);
					if (sample_time > blend_duration)
					{
						break;
					}
				}
				AnimationEngine::get_instance()->SampleClip(clip, sample_time, &data->skeleton_pose, true);

				float blend_duration = 0.0f;
				float blend_weight = 0.0f;
				if (GetNextBlend(i, instance->GetElaspedTime(), blend_weight, blend_duration))
				{
					int32_t next_index = i + 1;
					SkeletalAnimationChannel* next_channel = (SkeletalAnimationChannel*)m_channels[next_index];
					Ref<AnimationClip> next_clip = next_channel->GetAnimationClip();
					float next_sample_time = instance->GetElaspedTime() - next_channel->GetStartTime();
					if (next_clip)
					{
						AnimationEngine::get_instance()->SampleClip(next_clip, next_sample_time, &data->blend_pose, true);
						BlendPose(&data->skeleton_pose, &data->blend_pose, &data->skeleton_pose, blend_weight);
					}
				}
				data->skeleton_pose.SetEntityLocalPose();
				break;
			}
			else if (clip->HasRootMotion() && !IsTimeWithinChannel(sk_channel, instance->GetElaspedTime()))
			{
				//TODO: Check if logic is correct for blending
				if (IsNextWithinChannel(i))
				{
					float blend_duration = 0.0f;
					float blend_weight = 0.0f;
					GetNextBlend(i, instance->GetElaspedTime(), blend_weight, blend_duration);
					Transform curr_delta = AnimationEngine::get_instance()->GetRootMotionDelta(clip, data->skeletal_instance.GetSkeleton(), 0.0f,sk_channel->GetDuration() - blend_duration, true);
					total_pos_delta += curr_delta.GetPosition();
				}
				else
				{
					float blend_duration = 0.0f;
					float blend_weight = 0.0f;
					GetNextBlend(i, instance->GetElaspedTime(), blend_weight, blend_duration);
					Transform curr_delta = AnimationEngine::get_instance()->GetRootMotionDelta(clip, data->skeletal_instance.GetSkeleton(), 0.0f, sk_channel->GetDuration(), true);
					total_pos_delta += curr_delta.GetPosition();
				}
			}


		}
		root_motion_delta.SetPosition(total_pos_delta);
		Entity entity = scene->GetEntity(data->entity_id);
		Transform& pose = entity.GetComponent<TransformComponent>()._transform;
		Transform model_space_delta = Transform::CombineTransform_Direction(pose, root_motion_delta);
		pose.SetPosition(data->start_entity_position + model_space_delta.GetPosition());
	}

	bool ActivationTrack::Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{
		m_type = SequenceTrackType::ACTIVATION_TRACK;

		std::vector<void*>& tracks_data = instance->GetTracksData();

		RuntimeData* data = new RuntimeData; //TODO: Use custom allocator
		tracks_data[track_index] = data;

		data->current_channel_index = 0;

		Entity entity = scene->GetEntityByName(m_stringID);

		if (!entity)
		{
			TRC_ERROR("Invalid Entity, Function: {}", __FUNCTION__);
			return false;
		}

		data->entity_id = entity.GetID();
		data->enabled = false;


		return true;
	}

	ActivationTrack::ActivationTrack()
	{
		m_type = SequenceTrackType::ACTIVATION_TRACK;
	}

	void ActivationTrack::Update(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{

		std::vector<void*>& tracks_data = instance->GetTracksData();

		if (m_channels.empty())
		{
			return;
		}

		RuntimeData* data = (RuntimeData*)tracks_data[track_index];
		if (data->entity_id == 0)
		{
			return;
		}

		if (!IsTimeWithinChannel(m_channels[data->current_channel_index], instance->GetElaspedTime()))
		{
			int32_t index = FindChannelWithinTime(instance->GetElaspedTime(), data->current_channel_index);
			if (index < 0 && data->enabled)
			{
				scene->DisableEntity(scene->GetEntity(data->entity_id));
				data->enabled = false;
				return;
			}
			if (index < 0)
			{
				return;
			}
			data->current_channel_index = index;
		}
		if (IsTimeWithinChannel(m_channels[data->current_channel_index], instance->GetElaspedTime()) && !data->enabled)
		{
			scene->EnableEntity(scene->GetEntity(data->entity_id));
			data->enabled = true;
		}
		

	}

	bool SequenceTrack::IsTimeWithinChannel(SequenceTrackChannel* channel, float time)
	{
		float duration_in_sequence = channel->GetStartTime() + channel->GetDuration();
		if (time >= channel->GetStartTime() && time <= duration_in_sequence)
		{
			return true;
		}

		return false;
	}
	bool SequenceTrack::IsNextWithinChannel(int32_t channel_index)
	{
		if (channel_index >= m_channels.size())
		{
			return false;
		}

		if ((channel_index + 1) >= m_channels.size())
		{
			return false;
		}

		if (m_channels[channel_index]->GetEndTime() > m_channels[channel_index + 1]->GetStartTime())
		{
			return true;
		}

		return false;
	}
	bool SequenceTrack::IsPrevWithinChannel(int32_t channel_index)
	{
		if (channel_index >= m_channels.size())
		{
			return false;
		}

		if ((channel_index - 1) < 0)
		{
			return false;
		}

		if (m_channels[channel_index]->GetStartTime() < m_channels[channel_index - 1]->GetEndTime())
		{
			return true;
		}

		return false;
	}
	bool SequenceTrack::GetNextBlend(int32_t channel_index, float elapsed_time, float& out_blend_weight, float& out_blend_duration)
	{
		if (!IsNextWithinChannel(channel_index))
		{
			return false;
		}

		SequenceTrackChannel* channel = m_channels[channel_index];
		int32_t next_index = channel_index + 1;
		SequenceTrackChannel* next_channel = (AnimationChannel*)m_channels[next_index];
		float next_sample_time = elapsed_time - m_channels[next_index]->GetStartTime();
		float channel_end_time = channel->GetEndTime();
		out_blend_duration = channel_end_time - next_channel->GetStartTime();
		out_blend_weight = (elapsed_time - next_channel->GetStartTime()) / out_blend_duration;


		return true;
	}
	bool SequenceTrack::GetPrevBlend(int32_t channel_index, float elapsed_time, float& out_blend_weight, float& out_blend_duration)
	{
		if (!IsPrevWithinChannel(channel_index))
		{
			return false;
		}

		SequenceTrackChannel* channel = m_channels[channel_index];
		int32_t prev_index = channel_index - 1;
		SequenceTrackChannel* prev_channel = (AnimationChannel*)m_channels[prev_index];
		float prev_sample_time = elapsed_time - channel->GetStartTime();
		float channel_end_time = prev_channel->GetEndTime();
		out_blend_duration = channel_end_time - channel->GetStartTime();
		out_blend_weight = (elapsed_time - channel->GetStartTime()) / out_blend_duration;
		out_blend_weight = 1.0f - out_blend_weight;


		return true;
	}
	int32_t SequenceTrack::FindChannelWithinTime(float time, int32_t current_index)
	{
		float duration_in_sequence = m_channels[current_index]->GetStartTime() + m_channels[current_index]->GetDuration();
		if (time <= m_channels[current_index]->GetStartTime())
		{
			for (int32_t i = current_index; i >= 0; i--)
			{
				if (IsTimeWithinChannel(m_channels[i], time))
				{
					return i;
				}
			}
		}
		else if (time >= duration_in_sequence)
		{
			for (int32_t i = current_index; i < m_channels.size(); i++)
			{
				if (IsTimeWithinChannel(m_channels[i], time))
				{
					return i;
				}
			}
		}
		else if (IsTimeWithinChannel(m_channels[current_index], time))
		{
			return current_index;
		}

		return -1;
	}
	void SequenceTrack::RemoveChannel(int32_t index)
	{
		if (index > m_channels.size())
		{
			return;
		}

		SequenceTrackChannel* channel = m_channels[index];
		m_channels[index] = m_channels.back();
		m_channels.pop_back();

		delete channel;//TODO: Use custom allocator
	}

}
