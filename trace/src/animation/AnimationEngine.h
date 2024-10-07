#pragma once

#include "AnimationState.h"

namespace trace {
	class Scene;
	struct AnimationComponent;

	struct AnimatedOutput
	{
		char data[16];
		AnimationDataType type;
	};

	class AnimationEngine
	{

	public:

		bool Init();
		void Shutdown();

		void Animate(AnimationState& state, Scene* scene, std::unordered_map<std::string, UUID>& data_map);
		void Animate(Ref<AnimationClip> clip, Scene* scene, float time_point, std::unordered_map<std::string, UUID>& data_map);
		void Animate(AnimationComponent* animation_component, UUID entity, Scene* scene);

		static AnimationEngine* get_instance();
	private:

		void CalculateAndSetData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, Scene* scene, UUID id, AnimationDataType type, float time_point);
		AnimatedOutput CalculateData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, AnimationDataType type, float time_point);
		void SetData(AnimatedOutput& data, Scene* scene, UUID id);
		void FindFrame(std::unordered_map<std::string, UUID>& data_map, Ref<AnimationClip> clip, Scene* scene, float elasped_time, std::function<void(const std::string&,UUID , AnimationDataType ,AnimationFrameData* , AnimationFrameData* , float )> callback);

	protected:

	};

}
