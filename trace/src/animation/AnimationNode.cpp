#include "pch.h"

#include "animation/AnimationNode.h"
#include "animation/AnimationGraph.h"

namespace trace::Animation {
	bool EntryNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = new Result;
		instance_data_set[this] = result;
		result->entry_node = 0;
		

		return true;
	}

	// EntryNode -------------------------------------------------------------

	void EntryNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = reinterpret_cast<Result*>(instance_data_set[this]);

		int32_t index = 0;
		for (NodeInput& input : m_inputs)
		{
			if (input.node_id == 0)
			{
				continue;
			}

			Node* node = nodes[input.node_id];
			node->Update(instance, deltaTime);

			bool node_result = *(node->GetValue<bool>(instance, input.value_index));

			if (node_result)
			{
				result->entry_node = index;
				break;
			}

			index++;
		}
	}
	void* EntryNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = reinterpret_cast<Result*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &result->entry_node;
			break;
		}
		}

		return nullptr;
	}
	void EntryNode::Init(Graph* graph)
	{

	}

	void EntryNode::AddState(UUID node_id)
	{
		NodeInput input = {};
		input.node_id = 0;
		input.type = ValueType::Bool;
		input.value_index = INVALID_ID;

		m_inputs.push_back(input);
	}

	// -----------------------------------------------------------------------

	// Get Parameter ----------------------------------------------------
	bool GetParameterNode::Instanciate(GraphInstance* instance)
	{
		return true;
	}
	void GetParameterNode::Update(GraphInstance* instance, float deltaTime)
	{
	}
	void* GetParameterNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::vector<Parameter>& parameters = instance->GetGraph()->GetParameters();

		if (m_parameterIndex < 0 || m_parameterIndex > parameters.size() - 1)
		{
			return nullptr;
		}

		Parameter& param = parameters[m_parameterIndex];
		ParameterData* param_data = instance->GetParameterData(param);

		switch (value_index)
		{
		case 0:
		{
			return param_data->data;
			break;
		}
		}

		return nullptr;
	}
	void GetParameterNode::Init(Graph* graph)
	{
		NodeOutput output = {};
		output.type = ValueType::Unknown;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	void GetParameterNode::SetParameterIndex(int32_t index, Graph* graph)
	{
		std::vector<Parameter>& parameters = graph->GetParameters();

		if (index < 0 || index > parameters.size() - 1)
		{
			return;
		}

		Parameter& param = parameters[m_parameterIndex];
		m_parameterIndex = index;

		ValueType type = ValueType::Unknown;

		switch (param.second)
		{
		case ParameterType::Bool:
		{
			type = ValueType::Bool;
		}
		case ParameterType::Int:
		{
			type = ValueType::Int;
		}
		case ParameterType::Float:
		{
			type = ValueType::Float;
		}
		}

		m_outputs[0].value_index = 0;
		m_outputs[0].type = type;
	}

	// --------------------------------------------------------------------
}