#pragma once

#include "AnimationState.h"

namespace trace {
	class Scene;

	class AnimationEngine
	{

	public:

		bool Init();
		void Shutdown();

		void Animate(AnimationState& state, Scene* scene);

		static AnimationEngine* get_instance();
	private:
		static AnimationEngine* s_instance;

		void CalculateAndSetData(AnimationFrameData* a, AnimationFrameData* b, Scene* scene, UUID id, AnimationDataType type, float t);

	protected:

	};

}
