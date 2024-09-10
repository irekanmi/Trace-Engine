#pragma once

#include "AnimationState.h"

namespace trace {
	class Scene;

	class AnimationEngine
	{

	public:

		bool Init();
		void Shutdown();

		void Animate(AnimationState& state, Scene* scene, std::unordered_map<std::string, UUID>& data_map);
		void Animate(Ref<AnimationClip> clip, Scene* scene, float time_point, std::unordered_map<std::string, UUID>& data_map);

		static AnimationEngine* get_instance();
	private:

		void CalculateAndSetData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, Scene* scene, UUID id, AnimationDataType type, float time_point);

	protected:

	};

}
