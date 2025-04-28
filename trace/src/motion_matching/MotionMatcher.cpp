#include "pch.h"

#include "motion_matching/MotionMatchDatabase.h"
#include "MotionMatcher.h"
#include "core/io/Logging.h"

namespace trace::MotionMatching {



	bool MotionMatcher::Init(Ref<FeatureDatabase> database)
	{
		if (!database)
		{
			TRC_ERROR("Invalid database handle, Function: {}", __FUNCTION__);
			return false;
		}

		m_database = database;

		return true;
	}

	int32_t MotionMatcher::SearchPose(FeatureData& query, float trajectory_weight, float pose_weight, bool normalized_search)
	{
		if (!m_database)
		{
			TRC_ERROR("Invalid database handle, Function: {}", __FUNCTION__);
			return -1;
		}

		FeatureData search_query = {};
		if (normalized_search)
		{
			search_query = m_database->NormalizeFeature(query);
			search_query.joints = query.joints;
			search_query.root_velocity = query.root_velocity;
		}
		else
		{
			search_query = m_database->DenormalizeFeature(query);
			search_query.future_root_positions = query.future_root_positions;
			search_query.future_root_orientation = query.future_root_orientation;
		}

		int32_t best_index = -1;
		float cost = std::numeric_limits<float>::infinity();

		int32_t index = 0;
		FeatureData feature = {};
		for (FeatureData& _f : m_database->GetFeaturesData())
		{

			feature = normalized_search ? _f : m_database->DenormalizeFeature(_f);

			float pose_cost = 0.0f;
			pose_cost += glm::pow(glm::length(search_query.root_velocity - feature.root_velocity), 2.0f);

			for (int32_t i = 0; i < feature.joints.size(); i++)
			{
				pose_cost += glm::pow(glm::length(search_query.joints[i].position - feature.joints[i].position), 2.0f);
				pose_cost += glm::pow(glm::length(search_query.joints[i].velocity - feature.joints[i].velocity), 2.0f);

			}

			float trajectory_cost = 0.0f;
			for (int32_t i = 0; i < feature.future_root_positions.size(); i++)
			{
				trajectory_cost += glm::pow(glm::length(search_query.future_root_positions[i] - feature.future_root_positions[i]), 2.0f);
			}

			for (int32_t i = 0; i < feature.future_root_orientation.size(); i++)
			{
				if (normalized_search)
				{
					trajectory_cost += glm::pow(glm::length(search_query.future_root_orientation[i] - feature.future_root_orientation[i]), 2.0f);
				}
				else
				{
					trajectory_cost += glm::pow(glm::length(search_query.future_root_orientation[i] - feature.future_root_orientation[i]), 2.0f) * 300.0f;// NOTE: Because the magnitude of the orientation is small so it has to be scaled to as to allow the it to contribute significantly to the cost.

				}
			}

			float total_cost = (pose_cost * pose_weight) + (trajectory_cost * trajectory_weight);

			if (total_cost < cost)
			{
				cost = total_cost;
				best_index = index;
			}

			++index;
		}

		FeatureData* result = m_database->GetData(best_index);
		

		return best_index;
	}

}