#pragma once

#include "render/Transform.h"
#include "animation/Skeleton.h"
#include "core/Coretypes.h"
#include "scene/Scene.h"

#include <vector>

namespace trace::Animation {

	class Pose
	{

	public:

		bool Init(SkeletonInstance* skeleton);

		std::vector<Transform>& GetLocalPose() { return m_localPose; }
		std::vector<Transform>& GetGlobalPose();
		
		void SetEntityLocalPose();
		SkeletonInstance* GetSkeletonInstance() { return m_skeleton; }

	private:
		std::vector<Transform> m_localPose;
		std::vector<Transform> m_globalPose;
		SkeletonInstance* m_skeleton = nullptr;
		UpdateID m_updateID;



	protected:

	};
}
