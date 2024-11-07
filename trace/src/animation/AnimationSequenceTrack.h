#pragma once

#include "scene/UUID.h"
#include "core/Coretypes.h"
#include "animation/AnimationPose.h"

#include <vector>


namespace trace {

	class Scene;
	struct AnimationComponent;
}

namespace trace::Animation {

	class SequenceInstance;
	class SequenceTrackChannel;
	

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

	private:
	protected:
		std::vector<SequenceTrackChannel*> m_channels;
		StringID m_stringID = 0;

	};


	class AnimationSequenceTrack : public SequenceTrack
	{

	public:

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

	};

	class SkeletalAnimationTrack : public SequenceTrack
	{

	public:

		virtual bool Instanciate(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;
		virtual void Update(SequenceInstance* instance, Scene* scene, uint32_t track_index) override;

	private:

		struct RuntimeData
		{
			UUID entity_id = 0;
			int32_t current_channel_index = -1;
			AnimationComponent* anim_comp = nullptr;
			Pose skeleton_pose;
			Pose blend_pose;
		};

	protected:

	};

}
