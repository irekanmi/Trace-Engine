#pragma once

#include "AnimationState.h"
#include "resource/Resource.h"

#include <vector>

namespace trace {


	class AnimationGraph : public Resource
	{

	public:

		std::vector<AnimationState>& GetStates() { return m_states; }

		int currrent_state_index = 0;
	private:
		std::vector<AnimationState> m_states;

	protected:

	};

}
