#include "pch.h"

#include "core/maths/MathHelpers.h"
#include "core/io/Logging.h"

#include "glm/glm.hpp"
#include <limits>

namespace trace::MathHelpers {



	bool PointInCircle(Circle2D& circle, glm::vec2 point)
	{
		float distance = glm::length(circle.position - point);

		return distance <= circle.radius;
	}

	Circle2D GetTriangleCircumcircle(Triangle2D& triangle)
	{
		glm::vec2 p1 = triangle.vertex0;
		glm::vec2 p2 = triangle.vertex1;
		glm::vec2 p3 = triangle.vertex2;

		//Denominator
		float D1 = p1.x * (p2.y - p3.y);
		float D2 = p2.x * (p3.y - p1.y);
		float D3 = p3.x * (p1.y - p2.y);
		float D = 2 * (D1 + D2 + D3);

		if (glm::abs(D) < 1e-9)
		{
			TRC_ERROR("Points are collinear; no circumcircle exists, Function: {}", __FUNCTION__);
			return Circle2D();
		}

		float U_x1 = ((p1.x * p1.x) + (p1.y * p1.y)) * (p2.y - p3.y);
		float U_x2 = ((p2.x * p2.x) + (p2.y * p2.y)) * (p3.y - p1.y);
		float U_x3 = ((p3.x * p3.x) + (p3.y * p3.y)) * (p1.y - p2.y);

		float U_x = (1 / D) * (U_x1 + U_x2 + U_x3);

		float U_y1 = ((p1.x * p1.x) + (p1.y * p1.y)) * (p3.x - p2.x);
		float U_y2 = ((p2.x * p2.x) + (p2.y * p2.y)) * (p1.x - p3.x);
		float U_y3 = ((p3.x * p3.x) + (p3.y * p3.y)) * (p2.x - p1.x);

		float U_y = (1 / D) * (U_y1 + U_y2 + U_y3);

		float _x1 = U_x - p1.x;
		float _y1 = U_y - p1.y;
		
		float radius = glm::sqrt((_x1 * _x1) + (_y1 * _y1));

		Circle2D circle;
		circle.position = glm::vec2(U_x, U_y);
		circle.radius = radius;

		return circle;
	}
	
	void get_unique_edges(std::vector<Edge2D>& edges)
	{
		std::vector<uint32_t> bad_edges;
		for (uint32_t i = 0; i < edges.size(); i++)
		{
			Edge2D edge = edges[i];
			bool is_unique = true;
			for (uint32_t j = 0; j < edges.size(); j++)
			{
				if (i == j)
				{
					continue;
				}

				if (
					(edge.point0 == edges[j].point0 &&
						edge.point1 == edges[j].point1) ||
					(edge.point0 == edges[j].point1 &&
						edge.point1 == edges[j].point0)
					)
				{
					is_unique = false;
					break;
				}
			}
			
			if (!is_unique)
			{
				bad_edges.push_back(i);
			}
		}

		//Remove bad triangles
		uint32_t num_removed = 0;
		for (uint32_t bad_index : bad_edges)
		{
			uint32_t actual_index = bad_index - num_removed;

			edges.erase(edges.begin() + actual_index);

			num_removed++;
		}
	}

	bool DelaunayTrianglation(std::vector<glm::vec2>& points, std::vector<Triangle2D>& out_triangles)
	{

		if (points.size() <= 2)
		{
			TRC_ERROR("Number of points has to more than 2, Function: {}", __FUNCTION__);
			return false;
		}

		std::vector<Triangle2D> good_triangles;

		//Determine super triangle
		glm::vec2 min(std::numeric_limits<float>::infinity());
		glm::vec2 max(-std::numeric_limits<float>::infinity());
		for (uint32_t i = 0; i < points.size(); i++)
		{
			glm::vec2 point = points[i];
			
			min.x = point.x < min.x ? point.x : min.x;
			min.y = point.y < min.y ? point.y : min.y;
			
			max.x = point.x > max.x ? point.x : max.x;
			max.y = point.y > max.y ? point.y : max.y;
		}

		float dx = (max.x - min.x) * 10.0f;
		float dy = (max.y - min.y) * 10.0f;

		Triangle2D super_triangle;
		super_triangle.vertex0 = glm::vec2(min.x - dx, min.y - dy * 3.0f);
		super_triangle.vertex1 = glm::vec2(min.x - dx, max.y + dy);
		super_triangle.vertex2 = glm::vec2(max.x + dx * 3.0f, max.y + dy);

		good_triangles.push_back(super_triangle);

		for (uint32_t i = 0; i < points.size(); i++)
		{
			glm::vec2 point = points[i];

			std::vector<uint32_t> bad_triangles;
			std::vector<Edge2D> edges;
			for (uint32_t j = 0; j < good_triangles.size(); j++)
			{
				Triangle2D triangle = good_triangles[j];
				Circle2D circumcircle = GetTriangleCircumcircle(triangle);
				if (PointInCircle(circumcircle, point))
				{
					bad_triangles.push_back(j);
					Edge2D edge;
					edge.point0 = triangle.vertex0;
					edge.point1 = triangle.vertex1;
					edges.push_back(edge);

					edge.point0 = triangle.vertex1;
					edge.point1 = triangle.vertex2;
					edges.push_back(edge);

					edge.point0 = triangle.vertex0;
					edge.point1 = triangle.vertex2;
					edges.push_back(edge);
				}
			}
			//Remove bad triangles
			uint32_t num_removed = 0;
			for (uint32_t bad_index : bad_triangles)
			{
				uint32_t actual_index = bad_index - num_removed;

				good_triangles.erase(good_triangles.begin() + actual_index);

				num_removed++;
			}

			get_unique_edges(edges);

			for (Edge2D& edge : edges)
			{
				Triangle2D new_triangle;
				new_triangle.vertex0 = edge.point0;
				new_triangle.vertex1 = edge.point1;
				new_triangle.vertex2 = point;

				good_triangles.push_back(new_triangle);
			}
		}

		//Remove unused triangles
		std::vector<uint32_t> bad_indices;
		auto has_point = [&points](glm::vec2 p) -> bool {
			for (glm::vec2& point : points)
			{
				if (point == p)
				{
					return true;
				}

			}
			return false;
		};

		for (uint32_t i = 0; i < good_triangles.size(); i++)
		{
			Triangle2D triangle = good_triangles[i];
			
			if (
				!has_point(triangle.vertex0) ||
				!has_point(triangle.vertex1) ||
				!has_point(triangle.vertex2)
				)
			{
				bad_indices.push_back(i);
			}
		}
		uint32_t num_removed = 0;
		for (uint32_t bad_index : bad_indices)
		{
			uint32_t actual_index = bad_index - num_removed;

			good_triangles.erase(good_triangles.begin() + actual_index);

			num_removed++;
		}
		

		out_triangles = good_triangles;

		return true;
	}

	bool PointInTriangle(Triangle2D& triangle, glm::vec2 point, glm::vec3& out_weights)
	{
		glm::vec2 a = triangle.vertex0;
		glm::vec2 b = triangle.vertex1;
		glm::vec2 c = triangle.vertex2;

		float W_0 = a.x * (c.y - a.y);
		float W_1 = (point.y - a.y) * (c.x - a.x);
		float W_3 = point.x * (c.y - a.y);
		float W_4 = (b.y - a.y) * (c.x - a.x);
		float W_5 = (b.x - a.x) * (c.y - a.y);

		float w1 = (W_0 + W_1 - W_3) / (W_4 - W_5);

		float w2 = (point.y - a.y - (w1 * (b.y - a.y))) / (c.y - a.y);

		if (w1 >= 0.0f && w2 >= 0.0f && (w1 + w2) <= 1.0f)
		{
			float w3 = 1.0f - w1 - w2;
			out_weights = glm::vec3(w1, w2, w3);

			return true;
		}

		return false;
	}

}