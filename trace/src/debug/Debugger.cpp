#include "pch.h"

#include "Debugger.h"

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

	Debugger* Debugger::get_instance()
	{
		static Debugger* s_instance = new Debugger;
		return s_instance;
	}

}