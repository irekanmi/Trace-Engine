#pragma once

#include "motion_matching/MotionMatchDatabase.h"
#include "motion_matching/MotionMatcher.h"

namespace trace::MotionMatching {

	BEGIN_REGISTER_CLASS(JointData)
		REGISTER_TYPE(JointData);
		REGISTER_MEMBER(JointData, position);
		REGISTER_MEMBER(JointData, velocity);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(FeatureData)
		REGISTER_TYPE(FeatureData);
		REGISTER_MEMBER(FeatureData, joints);
		REGISTER_MEMBER(FeatureData, root_velocity);
		REGISTER_MEMBER(FeatureData, future_root_positions);
		REGISTER_MEMBER(FeatureData, future_root_orientation);
		REGISTER_MEMBER(FeatureData, clip_index);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(FeatureDatabase)
		REGISTER_TYPE(FeatureDatabase);
		REGISTER_MEMBER(FeatureDatabase, m_poseData);
		REGISTER_MEMBER(FeatureDatabase, m_animationIndex);
		REGISTER_MEMBER(FeatureDatabase, m_mean);
		REGISTER_MEMBER(FeatureDatabase, m_std);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(MotionMatcher)
		REGISTER_TYPE(MotionMatcher);
		REGISTER_MEMBER(MotionMatcher, m_database);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(MotionMatchingInfo)
		REGISTER_TYPE(MotionMatchingInfo);
	REGISTER_MEMBER(MotionMatchingInfo, pose_features);
	REGISTER_MEMBER(MotionMatchingInfo, trajectory_features);
	REGISTER_MEMBER(MotionMatchingInfo, frames_per_second);
	END_REGISTER_CLASS;

}
