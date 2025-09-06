#pragma once

#include "scene/UUID.h"
#include "core/Coretypes.h"
#include "animation/AnimationPose.h"
#include "reflection/TypeRegistry.h"
#include "animation/Skeleton.h"

#include <vector>


namespace trace {

	class Scene;
	struct AnimationGraphController;
}

namespace trace::Animation {

	class SequenceInstance;
	class SequenceTrackChannel;
	
	enum SequenceTrackType
	{
		UNKNOWN,
		ANIMATION_TRACK,
		SKELETAL_ANIMATION_TRACK,
		ACTIVATION_TRACK,
		CUSTOM_TRACK
	};

	class SequenceTrack
	{

	public:

		virtual ~SequenceTrack() {}
		virtual bool Instanciate(SequenceInstance* sequence, Scene* scene, uint32_t track_index) = 0;
		virtual void Update(SequenceInstance* sequence, Scene* scene, uint32_t track_index) = 0;
		std::vector<SequenceTrackChannel*>& GetTrackChannels() { return m_channels; }
		StringID GetStringID() { return m_stringID; }
		void SetStringID(StringID string_id) { m_stringID = string_id; }
		bool IsTimeWithinChannel(SequenceTrackChannel* channel, float time);
		bool IsNextWithinChannel(int32_t channel_index);
		bool IsPrevWithinChannel(int32_t channel_index);
		bool GetNextBlend(int32_t channel_index, float elapsed_time, float& out_blend_weight, float& out_blend_duration);
		bool GetPrevBlend(int32_t channel_index, float elapsed_time, float& out_blend_weight, float& out_blend_duration);
		int32_t FindChannelWithinTime(float time, int32_t current_index = 0);
		SequenceTrackType GetType() { return m_type; }
		void SetType(SequenceTrackType type) { m_type = type; }
		void RemoveChannel(int32_t index);

	private:
	protected:
		std::vector<SequenceTrackChannel*> m_channels;
		StringID m_stringID;
		SequenceTrackType m_type = SequenceTrackType::UNKNOWN;

		ACCESS_CLASS_MEMBERS(SequenceTrack);
		GET_TYPE_ID;

	};


	class AnimationSequenceTrack : public SequenceTrack
	{

	public:
		AnimationSequenceTrack();
		virtual ~AnimationSequenceTrack() {}
		virtual bool Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;
		virtual void Update(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;

	private:

		struct RuntimeData
		{
			int32_t current_channel_index = -1;
			UUID entity_id = 0;
			std::unordered_map<StringID, UUID> m_runtime_entites;
		};

	protected:

		ACCESS_CLASS_MEMBERS(AnimationSequenceTrack);
		GET_TYPE_ID;

	};

	class SkeletalAnimationTrack : public SequenceTrack
	{

	public:
		SkeletalAnimationTrack();
		virtual ~SkeletalAnimationTrack() {}
		virtual bool Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;
		virtual void Update(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;

	private:

		struct RuntimeData
		{
			UUID entity_id = 0;
			int32_t current_channel_index = -1;
			SkeletonInstance skeletal_instance;
			Pose skeleton_pose;
			Pose blend_pose;
			glm::vec3 start_entity_position;
			glm::quat start_entity_rotation;
		};

	protected:

		ACCESS_CLASS_MEMBERS(SkeletalAnimationTrack);
		GET_TYPE_ID;

	};

	class ActivationTrack : public SequenceTrack
	{

	public:
		ActivationTrack();
		virtual ~ActivationTrack() {}
		virtual bool Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;
		virtual void Update(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;

	private:

		struct RuntimeData
		{
			UUID entity_id = 0;
			int32_t current_channel_index = -1;
			bool enabled = false;
		};

	protected:

		ACCESS_CLASS_MEMBERS(ActivationTrack);
		GET_TYPE_ID;

	};

}
