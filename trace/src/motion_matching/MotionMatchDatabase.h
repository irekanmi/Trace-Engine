#pragma once

#include "animation/Animation.h"
#include "resource/Resource.h"
#include "reflection/TypeRegistry.h"
#include "scene/UUID.h"
#include "serialize/DataStream.h"
#include "resource/Ref.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include <unordered_map>

namespace trace::MotionMatching {

	struct MotionMatchingInfo
	{
		std::vector<Animation::HumanoidBone> pose_features;
		std::vector<int32_t> trajectory_features;
		int32_t frames_per_second;
	};

	struct JointData
	{
		glm::vec3 position;// zl
		//glm::quat rotation;// yr
		glm::vec3 velocity;// zv
		//glm::vec3 angular_velocity;// yw
	};

	struct FeatureData
	{
		// Pose Features
		std::vector<JointData> joints;
		//glm::quat root_rotation; // zr
		glm::vec3 root_velocity; // zv
		//glm::vec3 angular_velocity; // yv

		// Trajectory Feautures
		std::vector<glm::vec3> future_root_positions; //zp
		std::vector<glm::vec3> future_root_orientation; // zd

		int32_t clip_index = -1;
	};

	class FeatureDatabase : public Resource
	{

	public:

		bool ExtractPoseData(Ref<AnimationClip> clip, Ref<Animation::Skeleton> skeleton);
		void NormalizeDatabase();
		std::vector<FeatureData>& GetFeaturesData() { return m_poseData; }
		FeatureData* GetData(int32_t index);
		Ref<AnimationClip> GetAnimation(int32_t animation_index);
		std::unordered_map<int32_t, Ref<AnimationClip>>& GetAnimations() { return m_animationIndex; }
		virtual void Destroy() override;
		void Clear();
		FeatureData NormalizeFeature(FeatureData& feature);
		FeatureData DenormalizeFeature(FeatureData& feature);


		static Ref<FeatureDatabase> Deserialize(UUID id);
		static Ref<FeatureDatabase> Deserialize(DataStream* stream);

	private:
		std::vector<FeatureData> m_poseData;
		std::unordered_map<int32_t, Ref<AnimationClip>> m_animationIndex;
		FeatureData m_mean;
		FeatureData m_std;

	protected:
		ACCESS_CLASS_MEMBERS(FeatureDatabase);
		GET_TYPE_ID;


	};

}
