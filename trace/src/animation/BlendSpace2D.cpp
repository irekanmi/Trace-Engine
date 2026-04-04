#include "pch.h"

#include "animation/BlendSpace2D.h"
#include "animation/Animation.h"
#include "core/maths/MathHelpers.h"
#include "serialize/GenericSerializer.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "animation/AnimationPose.h"
#include "animation/Skeleton.h"
#include "animation/AnimationEngine.h"

#include <limits>


namespace trace {



	bool BlendSpace2D::Create()
	{
		if (!m_points.empty() && m_triangles.empty())
		{
			RebuildTriangulation();
		}

		return true;
	}

	void BlendSpace2D::Destroy()
	{
		m_animationMap.clear();
	}

	void BlendSpace2D::RebuildTriangulation()
	{
		if (m_points.size() <= 2)
		{
			TRC_ERROR("Number of points has to be more than 2, Function: {}", __FUNCTION__);
			return;
		}

		m_triangles.clear();

		std::vector<Triangle2D> triangles;
		MathHelpers::DelaunayTrianglation(m_points, triangles);

		m_min = glm::vec2(std::numeric_limits<float>::infinity());
		m_max = glm::vec2(-std::numeric_limits<float>::infinity());
		for (glm::vec2& point : m_points)
		{

			m_min.x = point.x < m_min.x ? point.x : m_min.x;
			m_min.y = point.y < m_min.y ? point.y : m_min.y;

			m_max.x = point.x > m_max.x ? point.x : m_max.x;
			m_max.y = point.y > m_max.y ? point.y : m_max.y;
		}

		

		auto find_point_index = [&](glm::vec2 p) -> uint32_t {
			for (uint32_t i = 0; i < m_points.size(); i++)
			{
				glm::vec2 point = m_points[i];
				if (point == p)
				{
					return i;
				}
			}

			return 0;//TODO: Find an invalid index to return
		};

		for (Triangle2D& triangle : triangles)
		{
			TrianglePointIndex triangle_index;
			triangle_index.triangle = triangle;

			triangle_index.index0 = find_point_index(triangle.vertex0);
			triangle_index.index1 = find_point_index(triangle.vertex1);
			triangle_index.index2 = find_point_index(triangle.vertex2);

			m_triangles.push_back(triangle_index);
		}

	}

	bool BlendSpace2D::GetPoseAt(Animation::Pose* result, Animation::Pose& pose_a, Animation::Pose& pose_b, Animation::Pose& pose_c, float x, float y, float sample_time)
	{
		if (!result)
		{
			TRC_ERROR("Please input a valid pointer, Function: {}", __FUNCTION__);
			return false;
		}

		glm::vec2 target_point = glm::clamp(glm::vec2(x, y), m_min, m_max);
		glm::vec3 out_weights;

		for (TrianglePointIndex& triangle : m_triangles)
		{
			if (!MathHelpers::PointInTriangle(triangle.triangle, target_point, out_weights))
			{
				continue;
			}

			Ref<AnimationClip> clip_a = GetPointAnimation(triangle.index0);
			if (!clip_a)
			{
				return false;
			}
			Ref<AnimationClip> clip_b = GetPointAnimation(triangle.index1);
			if (!clip_b)
			{
				return false;
			}
			Ref<AnimationClip> clip_c = GetPointAnimation(triangle.index2);
			if (!clip_c)
			{
				return false;
			}

			AnimationEngine::get_instance()->SampleClip(clip_a, sample_time, &pose_a, true);
			AnimationEngine::get_instance()->SampleClip(clip_b, sample_time, &pose_b, true);
			AnimationEngine::get_instance()->SampleClip(clip_c, sample_time, &pose_c, true);

			float weight_a = out_weights.x;
			float weight_b = out_weights.y;
			float weight_c = out_weights.z;

			std::vector<Transform>& source_pose_a = pose_a.GetLocalPose();

			for (uint32_t i = 0; i < source_pose_a.size(); i++)
			{
				Transform& p_a = pose_a.GetLocalPose()[i];
				Transform& p_b = pose_b.GetLocalPose()[i];
				Transform& p_c = pose_c.GetLocalPose()[i];

				glm::vec3 pos = (p_a.GetPosition() * weight_a) + (p_b.GetPosition() * weight_b) + (p_c.GetPosition() * weight_c);
				glm::quat rot = (p_a.GetRotation() * weight_a) + (p_b.GetRotation() * weight_b) + (p_c.GetRotation() * weight_c);
				rot = glm::normalize(rot);
				glm::vec3 scale = (p_a.GetScale() * weight_a) + (p_b.GetScale() * weight_b) + (p_c.GetScale() * weight_c);

				Transform& res = result->GetLocalPose()[i];
				res.SetPosition(pos);
				res.SetRotation(rot);
				res.SetScale(scale);
			}

			break;
		}

		

		return true;
	}


	Ref<AnimationClip> BlendSpace2D::GetPointAnimation(uint32_t point_index)
	{
		auto it = m_animationMap.find(point_index);
		if (it != m_animationMap.end())
		{
			return it->second;
		}

		return Ref<AnimationClip>();
	}

	Ref<BlendSpace2D> BlendSpace2D::Deserialize(UUID id)
	{
		Ref<BlendSpace2D> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<BlendSpace2D>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<BlendSpace2D>(id);
		}

		return result;
	}

	Ref<BlendSpace2D> BlendSpace2D::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<BlendSpace2D>(stream);
	}

}