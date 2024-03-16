#pragma once

#include "AnimationState.h"

#include <vector>

namespace trace {


	class AnimationGraph
	{

	public:

		std::vector<AnimationState>& GetStates() { return m_states; }

	private:
		std::vector<AnimationState> m_states;
		int m_currrentStateIndex = -1;

	protected:

	};

}
