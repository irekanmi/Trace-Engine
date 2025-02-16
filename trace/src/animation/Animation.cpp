#include "pch.h"

#include "scene/Entity.h"
#include "scene/Scene.h"
#include "animation/Animation.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Coretypes.h"
#include "external_utils.h"

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

		}

		return result;
	}

}
