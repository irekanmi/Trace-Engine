#pragma once

#include "animation/AnimationSequence.h"
#include "resource/Ref.h"

namespace trace {
	class Event;

	class AnimationSequencer
	{
	public:
		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float deltaTime);
		void OnEvent(Event* p_event);

		void SetAnimationSequence(Ref<Animation::Sequence> sequence);

	private:
		Animation::SequenceInstance m_instance;

	};

}