#include "pch.h"

#include "node_system/GenericGraph.h"
#include "core/io/Logging.h"
#include "spdlog/fmt/fmt.h"

#include "glm/glm.hpp"

namespace trace {

	void GenericGraph::DestroyGraph()
	{
		for (auto& node : m_nodes)
		{
			delete node.second;// TODO: Use custom allocator
		}
	}

	GenericNode* GenericGraph::GetNode(UUID node_id)
	{
		auto it = m_nodes.find(node_id);

		if (it != m_nodes.end())
		{
			return m_nodes[node_id];
		}

		TRC_ASSERT(false, "Ensure to Get valid nodes, Function: {}", __FUNCTION__);
		return nullptr;
	}

	void GenericGraph::DestroyNode(UUID node_id)
	{
		GenericNode* node = GetNode(node_id);
		if (node)
		{
			node->Destroy(this);
			m_nodes.erase(node_id);
			delete node;//TODO: Use custom allocator
		}
	}

	void GenericGraph::DestroyNodeWithInputs(UUID node_id)
	{
		GenericNode* node = GetNode(node_id);
		if (node)
		{
			node->Destroy(this);
			for (GenericNodeInput& input : node->GetInputs())
			{
				if (input.node_id != 0)
				{
					DestroyNode(input.node_id);
				}
			}
			m_nodes.erase(node_id);
			delete node;//TODO: Use custom allocator
		}
	}

	GenericParameterData* GenericGraphInstance::GetParameterData(GenericParameter& param)
	{
		int32_t index = m_parameterLUT[param.name];

		return &m_parameterData[index];
	}

	std::string GenericHelper::GetTypeString(GenericValueType type)
	{
		switch (type)
		{
		case GenericValueType::Float:
		{
			return "float";
			break;
		}
		case GenericValueType::Unknown:
		{
			return "unknown";
			break;
		}
		case GenericValueType::Int:
		{
			return "int";
			break;
		}
		case GenericValueType::Bool:
		{
			return "bool";
			break;
		}
		case GenericValueType::Vec2:
		{
			return "vec2";
			break;
		}
		case GenericValueType::Vec3:
		{
			return "vec3";
			break;
		}
		case GenericValueType::Vec4:
		{
			return "vec4";
			break;
		}
		case GenericValueType::Sampler2D:
		{
			return "sampler2D";
			break;
		}
		case GenericValueType::Execute:
		{
			return "execute";
			break;
		}
		}

		return std::string();
	}

	std::string GenericHelper::GetParameterValueString(GenericParameterData& in_data, GenericValueType type)
	{

		switch (type)
		{
		case GenericValueType::Float:
		{
			float value = 0;
			get_parameter_data(in_data, &value, sizeof(float));
			return std::to_string(value);
			break;
		}
		case GenericValueType::Int:
		{
			int value = 0;
			get_parameter_data(in_data, &value, sizeof(int));
			return std::to_string(value);
			break;
		}
		case GenericValueType::Bool:
		{
			bool value = 0;
			get_parameter_data(in_data, &value, sizeof(bool));
			return std::to_string(value);
			break;
		}
		case GenericValueType::Vec2:
		{
			glm::vec2 value = glm::vec2(0.0f, 0.0f);
			get_parameter_data(in_data, &value, sizeof(glm::vec2));
			return fmt::format("vec2({}, {})", value.x, value.y);
			break;
		}
		case GenericValueType::Vec3:
		{
			glm::vec3 value = glm::vec3(0.0f, 0.0f, 0.0f);
			get_parameter_data(in_data, &value, sizeof(glm::vec3));
			return fmt::format("vec3({}, {}, {})", value.x, value.y, value.z);
			break;
		}
		case GenericValueType::Vec4:
		{
			glm::vec4 value = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
			get_parameter_data(in_data, &value, sizeof(glm::vec4));
			return fmt::format("vec4({}, {}, {}, {})", value.x, value.y, value.z, value.w);
			break;
		}
		}

		return std::string();
	}

	void GenericHelper::get_parameter_data(GenericParameterData& in_data, void* out_value, uint32_t size)
	{
		memcpy(out_value, in_data.data, size);
	}

}

