#pragma once

#include "resource/Ref.h"
#include "resource/Resource.h"
#include "reflection/TypeRegistry.h"
#include "scene/UUID.h"
#include "serialize/DataStream.h"

#include <array>

namespace trace::Animation {

	// https://github.com/vrm-c/vrm-specification/blob/master/specification/VRMC_vrm-1.0-beta/humanoid.md#list-of-humanoid-bones
	enum class HumanoidBone
	{
		// Torso:
		Hips,			// Required
		Spine,			// Required
		Chest,
		UpperChest,
		Neck,

		// Head:
		Head,			// Required
		LeftEye,
		RightEye,
		Jaw,

		// Leg:
		LeftUpperLeg,	// Required
		LeftLowerLeg,	// Required
		LeftFoot,		// Required
		LeftToes,
		RightUpperLeg,	// Required
		RightLowerLeg,	// Required
		RightFoot,		// Required
		RightToes,

		// Arm:
		LeftShoulder,
		LeftUpperArm,	// Required
		LeftLowerArm,	// Required
		LeftHand,		// Required
		RightShoulder,
		RightUpperArm,	// Required
		RightLowerArm,	// Required
		RightHand,		// Required


		Count
	};

	class HumanoidRig : public Resource
	{

	public:

		HumanoidRig();
		virtual void Destroy() override;
		
		std::array<int32_t, (int)HumanoidBone::Count>& GetHumanoidBones() { return m_bones; }

	public:
		static Ref<HumanoidRig> Deserialize(UUID id);
		static Ref<HumanoidRig> Deserialize(DataStream* stream);

	private:
		std::array<int32_t, (int)HumanoidBone::Count> m_bones;
		

	protected:
		ACCESS_CLASS_MEMBERS(HumanoidRig);
	};

	const char* get_humanoid_bone_text(HumanoidBone bone);

}
