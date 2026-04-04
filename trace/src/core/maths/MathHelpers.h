#pragma once

#include "core/maths/Primitives.h"

#include <vector>

namespace trace::MathHelpers {

	bool PointInCircle(Circle2D& circle, glm::vec2 point);

	Circle2D GetTriangleCircumcircle(Triangle2D& triangle);

	bool DelaunayTrianglation(std::vector<glm::vec2>& points, std::vector<Triangle2D>& out_triangles);

	bool PointInTriangle(Triangle2D& triangle, glm::vec2 point, glm::vec3& out_weights);

}
