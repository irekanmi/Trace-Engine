#pragma once

#include "AnimationState.h"
#include "core/Coretypes.h"
#include "render/Transform.h"





namespace trace {
	class Scene;
	struct AnimationComponent;
	namespace Animation {
		class Pose;
	}
	
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

		void Animate(AnimationState& state, Scene* scene, std::unordered_map<StringID, UUID>& data_map);
		void Animate(Ref<AnimationClip> clip, Scene* scene, float time_point, bool loop, std::unordered_map<StringID, UUID>& data_map);
		void Animate(AnimationComponent* animation_component, Scene* scene, float time_point,  bool loop);
		void SampleClipWithRootMotionDelta(Ref<AnimationClip> clip, float from_time, float to_time, Animation::Pose* out_pose, bool looping);
		void SampleClip(Ref<AnimationClip> clip, float time, Animation::Pose* out_pose, bool looping);
		Transform GetRootMotionDelta(Ref<AnimationClip> clip, Ref<Skeleton> skeleton, float from_time, float to_time, bool looping);

		static AnimationEngine* get_instance();
	private:

		void CalculateAndSetData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, Scene* scene, UUID id, AnimationDataType type, float time_point);
		AnimatedOutput CalculateData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, AnimationDataType type, float time_point);
		void SetData(AnimatedOutput& data, Scene* scene, UUID id);
		void FindFrame(std::unordered_map<StringID, UUID>& data_map, Ref<AnimationClip> clip, Scene* scene, float elasped_time, std::function<void(StringID,UUID , AnimationDataType ,AnimationFrameData* , AnimationFrameData* , float )> callback);
		AnimatedOutput GetFrameData(Ref<AnimationClip> clip, AnimationDataTrack& channel, AnimationDataType type, float time, bool looping);

	protected:

	};

}
