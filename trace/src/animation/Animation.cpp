#include "pch.h"

#include "scene/Entity.h"
#include "scene/Scene.h"
#include "animation/Animation.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Coretypes.h"
#include "external_utils.h"
#include "resource/AnimationsManager.h"

namespace trace {

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

	bool AnimationClip::GenerateRootMotionData()
	{

		if (!m_hasRootMotion)
		{
			return false;
		}



		return true;
	}

	

	bool AnimationClip::GetRootMotionData(std::vector<Transform>& out_data, Ref<Animation::Skeleton> skeleton)
	{
		if (!HasRootMotion())
		{
			return false;
		}

		AnimationClip* clip = this;


		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

		auto& channel = clip->GetTracks()[bone->GetStringID()];
		auto& position_track = channel[AnimationDataType::POSITION];
		auto& rotation_track = channel[AnimationDataType::ROTATION];

		out_data.clear();
		out_data.resize(position_track.size());

		for (int32_t i = 0; i < position_track.size(); i++)
		{

			glm::vec3& pos = *(glm::vec3*)(position_track[i].data);
			glm::quat& rot = *(glm::quat*)(rotation_track[i].data);

			out_data[i] = Transform(pos, rot);
		}

		return true;
	}

	Ref<AnimationClip> AnimationClip::Deserialize(UUID id)
	{
		Ref<AnimationClip> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = AnimationsSerializer::DeserializeAnimationClip(file_path);
			}
		}
		else
		{
			return AnimationsManager::get_instance()->LoadClip_Runtime(id);
		}

		return result;
	}

	Ref<AnimationClip> AnimationClip::Deserialize(DataStream* stream)
	{
		return AnimationsSerializer::DeserializeAnimationClip(stream);
	}

}
