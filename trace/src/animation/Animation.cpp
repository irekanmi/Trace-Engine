#include "pch.h"

#include "scene/Entity.h"
#include "scene/Scene.h"
#include "animation/Animation.h"

namespace trace {
	void AnimationClip::SetAsRuntimeClip()
	{

		for (auto& track : m_tracks)
		{
			Transform pose;
			pose.SetDirty(false);
			m_runtimeTracks.insert(std::make_pair(track.first, pose));
		}

	}
	bool AnimationClip::Compare(AnimationClip* other)
	{
		if (!other)
		{
			return false;
		}

		bool rate_ = (m_sampleRate == other->m_sampleRate);
		bool duration_ = (m_duration == other->m_duration);
		bool id_ = (m_id == other->m_id);

		if (rate_ && duration_ && id_)
		{
			return true;
		}

		return false;
	}
	void AnimationPose::SetSkeleton(Skeleton* skeleton)
	{
		m_skeleton = skeleton;

		m_localPoses.clear();
		m_globalPoses.clear();

		m_localPoses.resize(skeleton->GetBones().size());
		m_globalPoses.resize(skeleton->GetBones().size());
		

	}
}
