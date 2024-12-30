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

		virtual bool Instanciate(SequenceInstance* sequence, Scene* scene, uint32_t track_index) = 0;
		virtual void Update(SequenceInstance* sequence, Scene* scene, uint32_t track_index) = 0;
		std::vector<SequenceTrackChannel*>& GetTrackChannels() { return m_channels; }
		StringID GetStringID() { return m_stringID; }
		void SetStringID(StringID string_id) { m_stringID = string_id; }
		bool IsTimeWithinChannel(SequenceTrackChannel* channel, float time);
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
		};

	protected:

		ACCESS_CLASS_MEMBERS(SkeletalAnimationTrack);
		GET_TYPE_ID;

	};

	class ActivationTrack : public SequenceTrack
	{

	public:
		ActivationTrack();
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
