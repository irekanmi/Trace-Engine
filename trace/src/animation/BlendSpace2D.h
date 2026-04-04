#pragma once

#include "core/Core.h"
#include "resource/Resource.h"
#include "resource/Ref.h"
#include "core/maths/Primitives.h"
#include "reflection/TypeRegistry.h"
#include "serialize/DataStream.h"

#include <vector>
#include <unordered_map>

namespace trace {

	class AnimationClip;

	namespace Animation {
		class Skeleton;
		class Pose;
	}
}

namespace trace {



	class BlendSpace2D : public Resource
	{

	public:
		struct TrianglePointIndex
		{
			uint32_t index0 = 0;
			uint32_t index1 = 0;
			uint32_t index2 = 0;

			Triangle2D triangle;
		};

		virtual ~BlendSpace2D() {}

		bool Create();
		virtual void Destroy() override;

		void RebuildTriangulation();
		bool GetPoseAt(Animation::Pose* result, Animation::Pose& pose_a, Animation::Pose& pose_b, Animation::Pose& pose_c, float x, float y, float sample_time);
		std::vector<TrianglePointIndex>& GetTriangles() { return m_triangles; }
		std::vector<glm::vec2>& GetPoints() { return m_points; }
		std::unordered_map<uint32_t, Ref<AnimationClip>>& GetAnimationMap() { return m_animationMap; }
		Ref<AnimationClip> GetPointAnimation(uint32_t point_index);

	private:

		std::vector<glm::vec2> m_points;
		std::vector<TrianglePointIndex> m_triangles;
		std::unordered_map<uint32_t, Ref<AnimationClip>> m_animationMap;
		glm::vec2 m_min;
		glm::vec2 m_max;

	public:
		static Ref<BlendSpace2D> Deserialize(UUID id);
		static Ref<BlendSpace2D> Deserialize(DataStream* stream);

	protected:
		ACCESS_CLASS_MEMBERS(BlendSpace2D);

	};

}