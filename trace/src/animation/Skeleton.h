#pragma once

#include "resource/Resource.h"
#include "animation/Bone.h"

#include <string>
#include <vector>

namespace trace {

	class Skeleton : public Resource
	{

	public:
		bool Create(const std::string& name, std::vector<Bone>& bones);
		virtual void Destroy() override;

		std::string& GetSkeletonName() { return m_name; }
		std::vector<Bone> GetBones() { return m_bones; }

	private:
		std::string m_name;
		std::vector<Bone> m_bones;

	protected:

	};

}
