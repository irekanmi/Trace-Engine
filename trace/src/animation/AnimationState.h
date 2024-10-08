#pragma once


#include "Animation.h"
#include "resource/Ref.h"

#include <string>

namespace trace {

	class AnimationTransition
	{

	};

	class Scene;

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
		// Anim State-> Play = 1, Paused = 2, Stop = 0
		uint8_t GetAnimState() { return m_playing; }
		float GetElaspedTime() { return m_elaspedTime; }
		AnimationClip& GetRuntimeClip() { return m_runtimeClip; }

		void SetName(const std::string& name) { m_name = name; }
		void SetAnimationClip(Ref<AnimationClip> clip);
		void SetLoop(bool loop) { m_loop = loop; }
		// Anim State-> Play = 1, Paused = 2, Stop = 0
		void SetAnimState(uint8_t state) { m_playing = state; }
		void SetElaspedTime(float value) { m_elaspedTime = value; }
		void SetAsRuntime();


	private:
		std::string m_name;
		Ref<AnimationClip> m_clip;
		AnimationClip m_runtimeClip;
		bool m_loop;
		float m_startTime = 0.0f;
		uint8_t m_playing = 0; // 0 == stoped or hasn't been played, 1 == playing, 2 == paused
		float m_elaspedTime = 0.0f;

	protected:

	};

}
