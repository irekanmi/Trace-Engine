#pragma once


#pragma once

#include "resource/Ref.h"
#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "FileStream.h"
#include "AssetsInfo.h"

#include <string>


namespace trace {

	class AnimationsSerializer
	{

	public:

		static bool SerializeAnimationClip(Ref<AnimationClip> clip, const std::string& file_path);
		static bool SerializeAnimationClip(Ref<AnimationClip> clip, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		static Ref<AnimationClip> DeserializeAnimationClip(const std::string& file_path);

		static bool SerializeAnimationGraph(Ref<AnimationGraph> graph, const std::string& file_path);
		static bool SerializeAnimationGraph(Ref<AnimationGraph> graph, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		static Ref<AnimationGraph> DeserializeAnimationGraph(const std::string& file_path);

	private:

	protected:

	};
}

