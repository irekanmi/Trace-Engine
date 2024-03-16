#pragma once


#include "Animation.h"
#include "resource/Ref.h"

#include <string>

namespace trace {

	class AnimationTransition
	{

	};


	class AnimationState
	{

	public:
		AnimationState();
		~AnimationState();

		void Play();
		void Pause();
		void Stop();
		bool IsPlaying();



		std::string& GetName() { return m_name; }
		Ref<AnimationClip> GetAnimationClip() { return m_clip; }
		bool GetLoop() { return m_loop; }
		float GetStartTime() { return m_startTime; }

		void SetName(const std::string& name) { m_name = name; }
		void SetAnimationClip(Ref<AnimationClip> clip) { m_clip = clip; }
		void SetLoop(bool loop) { m_loop = loop; }


	private:
		std::string m_name;
		Ref<AnimationClip> m_clip;
		bool m_loop;
		float m_startTime = 0.0f;
		uint8_t m_playing = 0; // 0 == stoped or hasn't been played, 1 == playing, 2 == paused
	protected:

	};

}
