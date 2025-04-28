#pragma once

#include "motion_matching/MotionMatchDatabase.h"
#include "reflection/TypeRegistry.h"

namespace trace::MotionMatching {

	class MotionMatcher
	{

	public:

		bool Init(Ref<FeatureDatabase> database);
		void SetDatabase(Ref<FeatureDatabase> database) { m_database = database; }
		Ref<FeatureDatabase> GetDatabase() { return m_database; }
		
		int32_t SearchPose(FeatureData& search_query, float trajectory_weight = 1.0f, float pose_weight = 1.0f, bool normalized_search = true);

	private:
		Ref<FeatureDatabase> m_database;

	protected:
		ACCESS_CLASS_MEMBERS(MotionMatcher);
		GET_TYPE_ID;

	};

}
