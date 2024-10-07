#pragma once

#include "AnimationState.h"
#include "resource/Resource.h"
#include "scene/UUID.h"

#include <vector>

namespace trace {
	class Scene;

	class AnimationGraph : public Resource
	{

	public:

		bool HasAnimationClip(Ref<AnimationClip> clip);

		std::vector<AnimationState>& GetStates() { return m_states; }
		int32_t GetCurrentStateIndex() { return m_currrentStateIndex; }
		int32_t GetStartIndex() { return m_startIndex; }
		AnimationState& GetCurrentState() { return m_states[m_currrentStateIndex]; }
		
		void SetAnimationStates(std::vector<AnimationState>& states) { m_states = std::move(states); }
		void SetCurrentStateIndex(int32_t current_index) { m_currrentStateIndex = current_index; }
		void SetStartIndex(int32_t start_index) { m_startIndex = start_index; }
		void SetAsRuntime();

		void Start();

		bool HasStarted() { return m_started; }

	private:
		int32_t m_currrentStateIndex = -1;
		int32_t m_startIndex = 0;
		std::vector<AnimationState> m_states;
		bool m_started = false;

	protected:

	};

}
