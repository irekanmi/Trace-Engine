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

namespace trace::Animation {


	bool AnimationSequenceTrack::Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{

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

	void AnimationSequenceTrack::Update(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{
		std::vector<void*>& tracks_data = instance->GetTracksData();

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
		AnimationEngine::get_instance()->Animate(clip, scene, sample_time, data->m_runtime_entites);

	}

	bool SkeletalAnimationTrack::Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{
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

		if (!entity.HasComponent<AnimationComponent>())
		{
			TRC_ERROR("Invalid Entity with no animation component, Function: {}", __FUNCTION__);
			return false;
		}

		data->entity_id = entity.GetID();
		data->anim_comp = &entity.GetComponent<AnimationComponent>();		
		GraphInstance& graph = data->anim_comp->graph_instance;
		if (!graph.HasStarted())
		{
			graph.Start(scene, data->entity_id);
		}
		data->skeleton_pose.Init(&graph.GetSkeletonInstance());
		data->blend_pose.Init(&graph.GetSkeletonInstance());


		return true;
	}

	void SkeletalAnimationTrack::Update(SequenceInstance* instance, Scene* scene, uint32_t track_index)
	{

		std::vector<void*>& tracks_data = instance->GetTracksData();

		RuntimeData* data = (RuntimeData*)tracks_data[track_index];

		if (!data->anim_comp)
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

		AnimationEngine::get_instance()->SampleClip(clip, sample_time, &data->skeleton_pose, true);

		int32_t next_index = data->current_channel_index + 1;
		if ( next_index < m_channels.size() && IsTimeWithinChannel(m_channels[next_index], instance->GetElaspedTime()))
		{
			
			AnimationChannel* next_channel = (AnimationChannel*)m_channels[next_index];
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
		
		data->skeleton_pose.SetEntityLocalPose();

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
	int32_t SequenceTrack::FindChannelWithinTime(float time, int32_t current_index)
	{
		float duration_in_sequence = m_channels[current_index]->GetStartTime() + m_channels[current_index]->GetDuration();
		if (time < m_channels[current_index]->GetStartTime())
		{
			for (int32_t i = current_index; i > 0; i--)
			{
				if (IsTimeWithinChannel(m_channels[i], time))
				{
					return i;
				}
			}
		}
		else if (time > duration_in_sequence)
		{
			for (int32_t i = current_index; i < m_channels.size(); i++)
			{
				if (IsTimeWithinChannel(m_channels[i], time))
				{
					return i;
				}
			}
		}

		return -1;
	}
}
