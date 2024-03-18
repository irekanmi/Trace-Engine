#pragma once


#pragma once

#include "resource/Ref.h"
#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include <string>


namespace trace {

	class AnimationsSerializer
	{

	public:

		static bool SerializeAnimationClip(Ref<AnimationClip> clip, const std::string& file_path);
		static Ref<AnimationClip> DeserializeAnimationClip(const std::string& file_path);

		static bool SerializeAnimationGraph(Ref<AnimationGraph> graph, const std::string& file_path);
		static Ref<AnimationGraph> DeserializeAnimationGraph(const std::string& file_path);

	private:

	protected:

	};
}

