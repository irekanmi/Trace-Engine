#include "pch.h"

#include "Debugger.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace trace {



	bool Debugger::Init()
	{
		m_initialized = true;

		return true;
	}

	void Debugger::Shutdown()
	{
		if (!m_initialized)
		{
			return;
		}

		m_initialized = false;
	}

	void Debugger::SetString(StringID string_id, const std::string& string_data)
	{
		m_stringData[string_id] = string_data;
	}

	std::string& Debugger::GetString(StringID string_id)
	{
		return m_stringData[string_id];
	}

	void Debugger::AddDebugLine(glm::vec3 point_0, glm::vec3 point_1, glm::mat4 transform, uint32_t color)
	{
		glm::vec4 from = transform * glm::vec4(point_0, 1.0f);
		glm::vec4 to = transform * glm::vec4(point_1, 1.0f);
		
		AddDebugLine(from, to, color);

	}

	void Debugger::AddDebugLine(glm::vec3 from, glm::vec3 to, uint32_t color)
	{
		float color_value = 0.0f;
		memcpy(&color_value, &color, sizeof(uint32_t));

		m_renderData.positions.emplace_back(glm::vec4(from, 0.0f));
		glm::vec4& line = m_renderData.positions.back();
		line.a = color_value;
		m_renderData.positions.emplace_back(glm::vec4(to, 0.0f));
		glm::vec4& next_line = m_renderData.positions.back();
		next_line.a = color_value;
		m_renderData.vert_count += 2;
	}

	void Debugger::DrawDebugCapsule(float radius, float height, glm::mat4 transform, uint32_t color)
	{
		DrawDebugCylinder(radius, height, transform, color);

		glm::mat4 new_transform = glm::translate(transform, glm::vec3(0.0f, height, 0.0f));
		DrawDebugHemiSphere(radius, 15, new_transform, color);

		new_transform = glm::translate(transform, glm::vec3(0.0f, -height, 0.0f));
		new_transform = glm::rotate(new_transform, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugHemiSphere(radius, 15, new_transform, color);
	}

	void Debugger::DrawDebugCylinder(float radius, float height, glm::mat4 transform, uint32_t color)
	{
		float x = radius;
		float z = 0.0f;
		float y = height;

		glm::vec3 point_1(x, y, z);
		y = -height;
		glm::vec3 point_2(x, y, z);
		//Line 1
		AddDebugLine(point_1, point_2, transform, color);

		x = -radius;
		z = 0.0f;
		y = height;
		point_1 = glm::vec3(x, y, z);
		y = -height;
		point_2 = glm::vec3(x, y, z);
		//Line 2
		AddDebugLine(point_1, point_2, transform, color);

		x = 0.0f;
		z = radius;
		y = height;
		point_1 = glm::vec3(x, y, z);
		y = -height;
		point_2 = glm::vec3(x, y, z);
		//Line 3
		AddDebugLine(point_1, point_2, transform, color);

		x = 0.0f;
		z = -radius;
		y = height;
		point_1 = glm::vec3(x, y, z);
		y = -height;
		point_2 = glm::vec3(x, y, z);
		//Line 4
		AddDebugLine(point_1, point_2, transform, color);

		glm::mat4 new_transform = glm::translate(transform, glm::vec3(0.0f, height, 0.0f));
		new_transform = glm::rotate(new_transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugCircle(radius, 15, new_transform, color);

		new_transform = glm::translate(transform, glm::vec3(0.0f, -height, 0.0f));
		new_transform = glm::rotate(new_transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugCircle(radius, 15, new_transform, color);

	}

	void Debugger::DrawDebugCircle(float radius, uint32_t steps, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
	{
		float ar = (glm::pi<float>() * 2.0f) / (float)steps;
		float theta = ar * (float)0;
		float x = radius * glm::cos(theta);
		float y = radius * glm::sin(theta);

		glm::vec4 previous_point = glm::vec4(x, y, 0.0f, 1.0f);
		for (uint32_t i = 1; i <= steps; i++)
		{
			float theta = ar * (float)i;
			float x = radius * glm::cos(theta);
			float y = radius * glm::sin(theta);

			glm::vec4 point = glm::vec4(x, y, 0.0f, 1.0f);
			AddDebugLine(glm::vec3(previous_point), glm::vec3(point), transform, color);
			previous_point = point;
		}
	}

	void Debugger::DrawDebugSemiCircle(float radius, uint32_t steps, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
	{
		float ar = (glm::pi<float>()) / (float)steps;
		float theta = ar * (float)0;
		float x = radius * glm::cos(theta);
		float y = radius * glm::sin(theta);

		glm::vec4 previous_point = glm::vec4(x, y, 0.0f, 1.0f);
		for (uint32_t i = 1; i <= steps; i++)
		{
			float theta = ar * (float)i;
			float x = radius * glm::cos(theta);
			float y = radius * glm::sin(theta);

			glm::vec4 point = glm::vec4(x, y, 0.0f, 1.0f);
			AddDebugLine(glm::vec3(previous_point), glm::vec3(point), transform, color);
			previous_point = point;
		}
	}

	void Debugger::DrawDebugSphere(float radius, uint32_t steps, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
	{
		DrawDebugCircle(radius, steps, transform, color, render_graph_index);
		glm::mat4 new_transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugCircle(radius, steps, new_transform, color, render_graph_index);
		new_transform = glm::rotate(transform, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugCircle(radius, steps, new_transform, color, render_graph_index);
		new_transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugCircle(radius, steps, new_transform, color, render_graph_index);
	}

	void Debugger::DrawDebugHemiSphere(float radius, uint32_t steps, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
	{
		DrawDebugSemiCircle(radius, steps, transform, color, render_graph_index);
		glm::mat4 new_transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugSemiCircle(radius, steps, new_transform, color, render_graph_index);
		new_transform = glm::rotate(transform, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugSemiCircle(radius, steps, new_transform, color, render_graph_index);
		new_transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugSemiCircle(radius, steps, new_transform, color, render_graph_index);
	}

	void Debugger::DrawDebugBox(float half_extent_x, float half_extent_y, float half_extent_z, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
	{
		glm::vec3 point1;
		glm::vec3 point2;

		float x, y, z;

		{
			x = half_extent_x;
			y = half_extent_y;
			z = half_extent_z;

			point1 = glm::vec3(x, y, z);
			z = -half_extent_z;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);

			x = half_extent_x;
			y = -half_extent_y;
			z = half_extent_z;
			point1 = glm::vec3(x, y, z);
			z = -half_extent_z;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);
		};

		{
			x = half_extent_x;
			y = half_extent_y;
			z = half_extent_z;
			point1 = glm::vec3(x, y, z);
			y = -half_extent_y;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);

			x = half_extent_x;
			y = half_extent_y;
			z = -half_extent_z;
			point1 = glm::vec3(x, y, z);
			y = -half_extent_y;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);
		};

		{
			x = -half_extent_x;
			y = half_extent_y;
			z = half_extent_z;

			point1 = glm::vec3(x, y, z);
			z = -half_extent_z;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);

			x = -half_extent_x;
			y = -half_extent_y;
			z = half_extent_z;
			point1 = glm::vec3(x, y, z);
			z = -half_extent_z;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);
		};

		{
			x = -half_extent_x;
			y = half_extent_y;
			z = half_extent_z;
			point1 = glm::vec3(x, y, z);
			y = -half_extent_y;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);

			x = -half_extent_x;
			y = half_extent_y;
			z = -half_extent_z;
			point1 = glm::vec3(x, y, z);
			y = -half_extent_y;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);
		};

		{
			x = half_extent_x;
			y = half_extent_y;
			z = half_extent_z;

			point1 = glm::vec3(x, y, z);
			x = -half_extent_x;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);

			x = half_extent_x;
			y = -half_extent_y;
			z = half_extent_z;
			point1 = glm::vec3(x, y, z);
			x = -half_extent_x;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);
		};

		{
			x = half_extent_x;
			y = half_extent_y;
			z = -half_extent_z;

			point1 = glm::vec3(x, y, z);
			x = -half_extent_x;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);

			x = half_extent_x;
			y = -half_extent_y;
			z = -half_extent_z;
			point1 = glm::vec3(x, y, z);
			x = -half_extent_x;
			point2 = glm::vec3(x, y, z);
			AddDebugLine(point1, point2, transform, color);
		};


	}

	Debugger* Debugger::get_instance()
	{
		static Debugger* s_instance = new Debugger;
		return s_instance;
	}

}