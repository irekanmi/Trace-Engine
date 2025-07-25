#pragma once


#pragma once

#include "resource/Ref.h"
#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "animation/AnimationSequence.h"
#include "FileStream.h"
#include "AssetsInfo.h"
#include "animation/HumanoidRig.h"

#include <string>


namespace trace {

	class AnimationsSerializer
	{

	public:

		static bool SerializeAnimationClip(Ref<AnimationClip> clip, const std::string& file_path);
		static bool SerializeAnimationClip(Ref<AnimationClip> clip, DataStream* stream);
		static bool SerializeAnimationClip(Ref<AnimationClip> clip, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		static Ref<AnimationClip> DeserializeAnimationClip(const std::string& file_path);
		static Ref<AnimationClip> DeserializeAnimationClip(DataStream* stream);
		static void DeserializeAnimationClip(Ref<AnimationClip> clip, FileStream& stream, AssetHeader& header);


		static bool SerializeSkeleton(Ref<Animation::Skeleton> skeleton, const std::string& file_path);
		static bool SerializeSkeleton(Ref<Animation::Skeleton> skeleton, DataStream* stream);
		static Ref<Animation::Skeleton> DeserializeSkeleton(const std::string& file_path);
		static Ref<Animation::Skeleton> DeserializeSkeleton(DataStream* stream);

		static bool SerializeAnimGraph(Ref<Animation::Graph> graph, const std::string& file_path);
		static bool SerializeAnimGraph(Ref<Animation::Graph> graph, DataStream* stream);
		static Ref<Animation::Graph> DeserializeAnimGraph(const std::string& file_path);
		static Ref<Animation::Graph> DeserializeAnimGraph(DataStream* stream);

		static bool SerializeSequence(Ref<Animation::Sequence> sequence, const std::string& file_path);
		static bool SerializeSequence(Ref<Animation::Sequence> sequence, DataStream* stream);
		static Ref<Animation::Sequence> DeserializeSequence(const std::string& file_path);
		static Ref<Animation::Sequence> DeserializeSequence(DataStream* stream);

		static bool SerializeHumanoidRig(Ref<Animation::HumanoidRig> sequence, const std::string& file_path);
		static bool SerializeHumanoidRig(Ref<Animation::HumanoidRig> sequence, DataStream* stream);
		static Ref<Animation::HumanoidRig> DeserializeHumanoidRig(const std::string& file_path);
		static Ref<Animation::HumanoidRig> DeserializeHumanoidRig(DataStream* stream);

		

	private:

	protected:

	};
}

