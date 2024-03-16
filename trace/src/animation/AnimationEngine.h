#pragma once

#include "AnimationState.h"

namespace trace {


	class AnimationEngine
	{

	public:

		bool Init();
		void Shutdown();

		void Animate(AnimationState& state);


	private:
	protected:

	};

}
