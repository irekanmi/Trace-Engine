#pragma once

#include "animation/Animation.h"
#include "core/Coretypes.h"
#include "reflection/TypeRegistry.h"

#include <vector>

namespace trace {

	class Scene;

}

namespace trace::Animation {

	class SequenceInstance;
	class SequenceTrack;
	

	class SequenceTrackChannel
	{

	public:

		virtual void OnStart() {}
		virtual void Update(SequenceInstance* sequence, Scene* scene, uint32_t track_index) {}
		virtual void OnEnd() {}

		float GetStartTime() { return m_startTime; }
		float GetDuration() { return m_duration; }

		void SetStartTime(float start_time) { m_startTime = start_time; }
		void SetDuration(float duration) { m_duration = duration; }

	private:
	protected:
		float m_startTime = 0.0f;
		float m_duration = 0.0f;

		ACCESS_CLASS_MEMBERS(SequenceTrackChannel);
		GET_TYPE_ID;

	};


	class AnimationChannel : public SequenceTrackChannel
	{

	public:

		//void GetAnimationData(SequenceInstance* instance, Scene* scene, AnimationChannelData& final_data);

		Ref<AnimationClip> GetAnimationClip() { return m_clip; }
		void SetAnimationClip(Ref<AnimationClip> clip) { m_clip = clip; }

	private:
		Ref<AnimationClip> m_clip;


	protected:
		ACCESS_CLASS_MEMBERS(AnimationChannel);
		GET_TYPE_ID;

	};

	class SkeletalAnimationChannel : public SequenceTrackChannel
	{

	public:
		
		Ref<AnimationClip> GetAnimationClip() { return m_clip; }
		void SetAnimationClip(Ref<AnimationClip> clip) { m_clip = clip; }

	private:
		Ref<AnimationClip> m_clip;


	protected:
		ACCESS_CLASS_MEMBERS(SkeletalAnimationChannel);
		GET_TYPE_ID;

	};

}
