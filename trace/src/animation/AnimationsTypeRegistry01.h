#pragma once

#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "animation/AnimationNode.h"
#include "animation/AnimationPoseNode.h"
#include "animation/AnimationSequence.h"
#include "animation/AnimationSequenceTrack.h"
#include "animation/SequenceTrackChannel.h"
#include "animation/Bone.h"
#include "animation/Skeleton.h"
#include "animation/HumanoidRig.h"

#include "reflection/TypeRegistry.h"

#include "scene/Entity.h"
#include "scene/Scene.h"

namespace trace {

	REGISTER_TYPE(AnimationDataType);
	REGISTER_TYPE(AnimationClipType);

	BEGIN_REGISTER_CLASS(AnimationFrameData)
		REGISTER_TYPE(AnimationFrameData);
	REGISTER_MEMBER(AnimationFrameData, data);
	REGISTER_MEMBER(AnimationFrameData, time_point);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(RootMotionInfo)
		REGISTER_TYPE(RootMotionInfo);
	REGISTER_MEMBER(RootMotionInfo, Y_motion);
	REGISTER_MEMBER(RootMotionInfo, XZ_motion);
	REGISTER_MEMBER(RootMotionInfo, enable_rotation);
	REGISTER_MEMBER(RootMotionInfo, root_bone_index);
	END_REGISTER_CLASS;

	REGISTER_CONTAINER(std::vector, AnimationFrameData);
	REGISTER_KEY_VALUE_CONTAINER_PLACEHOLDER(std::unordered_map, AnimationDataType, std::vector<AnimationFrameData>, AnimationDataTrack);

	BEGIN_REGISTER_CLASS(AnimationClip)
		REGISTER_TYPE(AnimationClip);
	REGISTER_MEMBER(AnimationClip, m_duration);
	REGISTER_MEMBER(AnimationClip, m_sampleRate);
	REGISTER_MEMBER(AnimationClip, m_tracks);
	REGISTER_MEMBER(AnimationClip, m_type);
	REGISTER_MEMBER(AnimationClip, m_hasRootMotion);
	REGISTER_MEMBER(AnimationClip, m_rootMotionInfo);
	END_REGISTER_CLASS;

}
