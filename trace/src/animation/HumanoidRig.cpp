#include "pch.h"

#include "HumanoidRig.h"
#include "serialize/AnimationsSerializer.h"
#include "resource/GenericAssetManager.h"
#include "external_utils.h"

namespace trace::Animation {



	const char* get_humanoid_bone_text(HumanoidBone bone)
	{
		static const char* data[] = {
		"Hips",			
		"Spine",			
		"Chest",
		"UpperChest",
		"Neck",
		"Head",			
		"LeftEye",
		"RightEye",
		"Jaw",
		"LeftUpperLeg",	
		"LeftLowerLeg",	
		"LeftFoot",		
		"LeftToes",
		"RightUpperLeg",	
		"RightLowerLeg",	
		"RightFoot",		
		"RightToes",
		"LeftShoulder",
		"LeftUpperArm",	
		"LeftLowerArm",	
		"LeftHand",		
		"RightShoulder",
		"RightUpperArm",	
		"RightLowerArm",	
		"RightHand",		
		};

		return data[(int32_t)bone];
	}

	HumanoidRig::HumanoidRig()
	{
		m_bones.fill(-1);
	}

	void HumanoidRig::Destroy()
	{
	}

	Ref<HumanoidRig> HumanoidRig::Deserialize(UUID id)
	{
		Ref<HumanoidRig> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = AnimationsSerializer::DeserializeHumanoidRig(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<HumanoidRig>(id);
		}

		return result;
	}

	Ref<HumanoidRig> HumanoidRig::Deserialize(DataStream* stream)
	{
		return AnimationsSerializer::DeserializeHumanoidRig(stream);
	}

}