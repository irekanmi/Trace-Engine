#include "pch.h"

#include "motion_matching/MotionMatchDatabase.h"
#include "core/io/Logging.h"
#include "animation/AnimationEngine.h"
#include "animation/AnimationPose.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "serialize/GenericSerializer.h"



namespace trace::MotionMatching {



	bool FeatureDatabase::ExtractPoseData(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton)
	{
		if (!clip)
		{
			TRC_ERROR("Invalid Animation Clip Handle, Function: {}", __FUNCTION__);
			return false;
		}

		if (!skeleton)
		{
			TRC_ERROR("Invalid Skeleton Handle, Function: {}", __FUNCTION__);
			return false;
		}

		if (!skeleton->GetHumanoidRig())
		{
			TRC_ERROR("Skeleton doesn't have a humanoid rig  skeleton name: {}, Function: {}", skeleton->GetName(), __FUNCTION__);
			return false;
		}

		if (!clip->HasRootMotion())
		{
			TRC_ERROR("Animation Clip doesn't have root motion animation clip name: {}, Function: {}", clip->GetName(), __FUNCTION__);
			return false;
		}

		if (!m_motionMatchingInfo)
		{
			TRC_ERROR("Invalid Motion Matching Info Handle, Function: {}", __FUNCTION__);
			return false;
		}

		//TODO: Check if there is a better way to save animation clip index
		int32_t clip_index = m_poseData.size();
		m_animationIndex[clip_index] = clip;

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Ref<Animation::HumanoidRig> rig = skeleton->GetHumanoidRig();
				
		Animation::Bone* root_bone = skeleton->GetBone(root_motion_info.root_bone_index);
		auto& channel = clip->GetTracks()[root_bone->GetStringID()];

		auto& position_track = channel[AnimationDataType::POSITION];

		float clip_duration = clip->GetDuration();
		float dt = 1.0f / float(clip->GetSampleRate());


		
		//First animation frame
		{
			AnimationFrameData& frame_data = position_track[0];
			AnimationFrameData& prev_frame_data = position_track[position_track.size() - 1];

			Animation::Pose frame_pose;
			frame_pose.Init(skeleton);
			AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, frame_data.time_point, &frame_pose, true);
			Transform& root_data = frame_pose.GetRootMotionDelta();

			Animation::Pose prev_frame_pose;
			prev_frame_pose.Init(skeleton);
			AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, prev_frame_data.time_point, &prev_frame_pose, true);
			Transform& prev_root_data = prev_frame_pose.GetRootMotionDelta();

			FeatureData frame_feature;

			// Pose Features =====================================================
			frame_feature.root_velocity = (root_data.GetPosition() - prev_root_data.GetPosition()) / dt;

			for (auto& bone : m_motionMatchingInfo->pose_features)
			{
				if ((int32_t)bone < 0)
				{
					continue;
				}

				Transform _transform = frame_pose.GetBoneGlobalPose(skeleton, (int32_t)bone);
				Transform _prev_transform = prev_frame_pose.GetBoneGlobalPose(skeleton, (int32_t)bone);

				JointData _data = {};
				_data.position = _transform.GetPosition();
				_data.velocity = (_transform.GetPosition() - _prev_transform.GetPosition()) / dt;

				frame_feature.joints.push_back(_data);
			}

			// ================================================

			// Tracjectory features ----------------------------------------------------------

			for (int32_t& _frame_index : m_motionMatchingInfo->trajectory_features)
			{
				Animation::Pose _frame_pose_x;
				_frame_pose_x.Init(skeleton);

				float _frame_x = float(_frame_index) / float(m_motionMatchingInfo->frames_per_second);
				AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, frame_data.time_point + _frame_x, &_frame_pose_x, true);

				Transform& root_data_x = _frame_pose_x.GetRootMotionDelta();

				glm::vec3 position_x = root_data_x.GetPosition() - root_data.GetPosition();
				position_x = glm::inverse(root_data.GetRotation()) * position_x;
				frame_feature.future_root_positions.push_back(position_x);

				glm::vec3 orientation_x = glm::inverse(root_data.GetRotation()) * (root_data_x.GetForward());
				frame_feature.future_root_orientation.push_back(orientation_x);
			}

			// ---------------------------------------------------------------------------

			frame_feature.clip_index = clip_index;

			m_poseData.push_back(frame_feature);
		};


		for (int32_t i = 1; i < position_track.size(); i++)
		{
			AnimationFrameData& frame_data = position_track[i];
			AnimationFrameData& prev_frame_data = position_track[i - 1];

			Animation::Pose frame_pose;
			frame_pose.Init(skeleton);
			AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, frame_data.time_point, &frame_pose, true);
			Transform& root_data = frame_pose.GetRootMotionDelta();

			Animation::Pose prev_frame_pose;
			prev_frame_pose.Init(skeleton);
			AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, prev_frame_data.time_point, &prev_frame_pose, true);
			Transform& prev_root_data = prev_frame_pose.GetRootMotionDelta();

			FeatureData frame_feature;

			// Pose Features =====================================================
			frame_feature.root_velocity = (root_data.GetPosition() - prev_root_data.GetPosition()) / dt;

			for (auto& bone : m_motionMatchingInfo->pose_features)
			{
				if ((int32_t)bone < 0)
				{
					continue;
				}

				Transform _transform = frame_pose.GetBoneGlobalPose(skeleton, (int32_t)bone);
				Transform _prev_transform = prev_frame_pose.GetBoneGlobalPose(skeleton, (int32_t)bone);

				JointData _data = {};
				_data.position = _transform.GetPosition();
				_data.velocity = (_transform.GetPosition() - _prev_transform.GetPosition()) / dt;

				frame_feature.joints.push_back(_data);
			}
			
			// ================================================

			// Tracjectory features ----------------------------------------------------------

			for (int32_t& _frame_index : m_motionMatchingInfo->trajectory_features)
			{
				Animation::Pose _frame_pose_x;
				_frame_pose_x.Init(skeleton);

				float _frame_x = float(_frame_index) / float(m_motionMatchingInfo->frames_per_second);
				AnimationEngine::get_instance()->SampleClipWithRootMotion(clip, skeleton, frame_data.time_point + _frame_x, &_frame_pose_x, true);

				Transform& root_data_x = _frame_pose_x.GetRootMotionDelta();

				glm::vec3 position_x = root_data_x.GetPosition() - root_data.GetPosition();
				position_x = glm::inverse(root_data.GetRotation()) * position_x;
				frame_feature.future_root_positions.push_back(position_x);

				glm::vec3 orientation_x = glm::inverse(root_data.GetRotation()) * (root_data_x.GetForward());
				frame_feature.future_root_orientation.push_back(orientation_x);
			}

			// ---------------------------------------------------------------------------

			frame_feature.clip_index = clip_index;

			m_poseData.push_back(frame_feature);


		}


		return true;
	}

	void FeatureDatabase::NormalizeDatabase()
	{
		size_t feature_count = m_poseData.size();
		float inv_count = 1.0f / float(feature_count);

		m_mean = {};
		m_std = {};

		// Compute mean
		for (FeatureData& feature : m_poseData)
		{
			m_mean.root_velocity += feature.root_velocity;
			if (m_mean.joints.empty())
			{
				m_mean.joints.resize(feature.joints.size());
			}

			for (int32_t i = 0; i < feature.joints.size(); i++)
			{
				m_mean.joints[i].position += feature.joints[i].position;
				m_mean.joints[i].velocity += feature.joints[i].velocity;
			}

			if (m_mean.future_root_positions.empty())
			{
				m_mean.future_root_positions.resize(feature.future_root_positions.size());
			}

			for (int32_t i = 0; i < feature.future_root_positions.size(); i++)
			{
				m_mean.future_root_positions[i] += feature.future_root_positions[i];
			}

			if (m_mean.future_root_orientation.empty())
			{
				m_mean.future_root_orientation.resize(feature.future_root_orientation.size());
			}

			for (int32_t i = 0; i < feature.future_root_orientation.size(); i++)
			{
				m_mean.future_root_orientation[i] += feature.future_root_orientation[i];
			}


		}

		m_mean.root_velocity *= inv_count;

		for (JointData& jnt : m_mean.joints)
		{
			jnt.position *= inv_count;
			jnt.velocity *= inv_count;
		}

		for (glm::vec3& future_pos : m_mean.future_root_positions)
		{
			future_pos *= inv_count;
		}
		
		for (glm::vec3& future_dir : m_mean.future_root_orientation)
		{
			future_dir *= inv_count;
		}

		// Compute Standard deviation
		for (FeatureData& feature : m_poseData)
		{
			m_std.root_velocity += glm::pow(feature.root_velocity - m_mean.root_velocity, glm::vec3(2.0f));
			
			if (m_std.joints.empty())
			{
				m_std.joints.resize(feature.joints.size());
			}

			for (int32_t i = 0; i < feature.joints.size(); i++)
			{
				m_std.joints[i].position += glm::pow(feature.joints[i].position - m_mean.joints[i].position, glm::vec3(2.0f));
				m_std.joints[i].velocity += glm::pow(feature.joints[i].velocity - m_mean.joints[i].velocity, glm::vec3(2.0f));
			}

			if (m_std.future_root_positions.empty())
			{
				m_std.future_root_positions.resize(feature.future_root_positions.size());
			}

			for (int32_t i = 0; i < feature.future_root_positions.size(); i++)
			{
				m_std.future_root_positions[i] += glm::pow(feature.future_root_positions[i] - m_mean.future_root_positions[i], glm::vec3(2.0f));
			}

			if (m_std.future_root_orientation.empty())
			{
				m_std.future_root_orientation.resize(feature.future_root_orientation.size());
			}

			for (int32_t i = 0; i < feature.future_root_orientation.size(); i++)
			{
				m_std.future_root_orientation[i] += glm::pow(feature.future_root_orientation[i] - m_mean.future_root_orientation[i], glm::vec3(2.0f));
			}


		}

		m_std.root_velocity = glm::sqrt(m_std.root_velocity * inv_count);

		for (JointData& jnt : m_std.joints)
		{
			jnt.position = glm::sqrt(jnt.position * inv_count);
			jnt.velocity = glm::sqrt(jnt.velocity * inv_count);
		}

		for (glm::vec3& future_pos : m_std.future_root_positions)
		{
			future_pos = glm::sqrt(future_pos * inv_count);
		}

		for (glm::vec3& future_dir : m_std.future_root_orientation)
		{
			future_dir = glm::sqrt(future_dir * inv_count);
		}

		// Avoid division by zero (clamp to 1.0f if standard deviation is too small
		auto safe = [](glm::vec3& val)
		{
			for (int32_t i = 0; i < 3; i++)
			{
				val[i] = val[i] < 1e-6f ? 1.0f : val[i];
			}
		};

		safe(m_std.root_velocity);

		for (JointData& jnt : m_std.joints)
		{
			safe(jnt.position);
			safe(jnt.velocity);
		}

		for (glm::vec3& future_pos : m_std.future_root_positions)
		{
			safe(future_pos);
		}

		for (glm::vec3& future_dir : m_std.future_root_orientation)
		{
			safe(future_dir);
		}


		// Normalize database
		for (FeatureData& feature : m_poseData)
		{
			feature = NormalizeFeature(feature);
		}

	}

	Ref<AnimationClip> FeatureDatabase::GetAnimation(int32_t animation_index)
	{
		auto it = m_animationIndex.find(animation_index);
		if (it != m_animationIndex.end())
		{
			return it->second;
		}
		return Ref<AnimationClip>();
	}

	FeatureData* FeatureDatabase::GetData(int32_t index)
	{
		if (index < 0 || index > m_poseData.size() - 1)
		{
			TRC_ERROR("Invalid index, Function: {}", __FUNCTION__);
			return nullptr;
		}

		return &m_poseData[index];
	}



	FeatureData FeatureDatabase::NormalizeFeature(FeatureData& feature)
	{
		FeatureData result = feature;
		
		result.root_velocity = (feature.root_velocity - m_mean.root_velocity) / m_std.root_velocity;

		for (int32_t i = 0; i < feature.joints.size(); i++)
		{
			JointData& jnt = feature.joints[i];
			result.joints[i].position = (jnt.position - m_mean.joints[i].position) / m_std.joints[i].position;
			result.joints[i].velocity = (jnt.velocity - m_mean.joints[i].velocity) / m_std.joints[i].velocity;
		}

		for (int32_t i = 0; i < feature.future_root_positions.size(); i++)
		{
			result.future_root_positions[i] = (feature.future_root_positions[i] - m_mean.future_root_positions[i]) / m_std.future_root_positions[i];
		}

		for (int32_t i = 0; i < feature.future_root_orientation.size(); i++)
		{
			result.future_root_orientation[i] = (feature.future_root_orientation[i] - m_mean.future_root_orientation[i]) / m_std.future_root_orientation[i];
		}
		
		return result;
	}
	
	FeatureData FeatureDatabase::DenormalizeFeature(FeatureData& feature)
	{
		FeatureData result = feature;
		
		result.root_velocity = (feature.root_velocity * m_std.root_velocity) + m_mean.root_velocity;

		for (int32_t i = 0; i < feature.joints.size(); i++)
		{
			JointData& jnt = feature.joints[i];
			result.joints[i].position = (jnt.position * m_std.joints[i].position) + m_mean.joints[i].position;
			result.joints[i].velocity = (jnt.velocity * m_std.joints[i].velocity) + m_mean.joints[i].velocity;
		}

		for (int32_t i = 0; i < feature.future_root_positions.size(); i++)
		{
			result.future_root_positions[i] = (feature.future_root_positions[i] * m_std.future_root_positions[i]) + m_mean.future_root_positions[i];
		}

		for (int32_t i = 0; i < feature.future_root_orientation.size(); i++)
		{
			result.future_root_orientation[i] = (feature.future_root_orientation[i] * m_std.future_root_orientation[i]) + m_mean.future_root_orientation[i];
		}
		
		return result;
	}
	
	void FeatureDatabase::Destroy()
	{
		Clear();
	}
	
	void FeatureDatabase::Clear()
	{
		m_poseData.clear();
		m_animationIndex.clear();
	}

	Ref<FeatureDatabase> FeatureDatabase::Deserialize(UUID id)
	{
		Ref<FeatureDatabase> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<FeatureDatabase>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<FeatureDatabase>(id);
		}

		return result;
	}

	Ref<FeatureDatabase> FeatureDatabase::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<FeatureDatabase>(stream);
	}

	Ref<MotionMatchingInfo> MotionMatchingInfo::Deserialize(UUID id)
	{
		Ref<MotionMatchingInfo> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<MotionMatchingInfo>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<MotionMatchingInfo>(id);
		}

		return result;
	}

	void MotionMatchingInfo::Destroy()
	{
		
	}

	Ref<MotionMatchingInfo> MotionMatchingInfo::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<MotionMatchingInfo>(stream);
	}

}