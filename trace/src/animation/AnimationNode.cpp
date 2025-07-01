#include "pch.h"

#include "animation/AnimationNode.h"
#include "animation/AnimationGraph.h"
#include "networking/NetworkStream.h"

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

	void EntryNode::Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = reinterpret_cast<Result*>(instance_data_set[this]);

		for (NodeInput& input : m_inputs)
		{
			if (input.node_id == 0)
			{
				continue;
			}

			result->entry_node = input.node_id;
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
		NodeInput input = {};
		input.node_id = 0;
		input.type = ValueType::Unknown;
		input.value_index = 0;
	}


	// -----------------------------------------------------------------------

	// Get Parameter ----------------------------------------------------
	bool GetParameterNode::Instanciate(GraphInstance* instance)
	{
		return true;
	}
	void GetParameterNode::Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream)
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
			break;
		}
		case ParameterType::Int:
		{
			type = ValueType::Int;
			break;
		}
		case ParameterType::Float:
		{
			type = ValueType::Float;
			break;
		}
		}

		m_outputs[0].value_index = 0;
		m_outputs[0].type = type;
	}

	bool IfNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = new Result;
		instance_data_set[this] = result;

		return true;
	}

	void IfNode::Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = reinterpret_cast<Result*>(instance_data_set[this]);

		NodeInput& input = m_inputs[0];
		if (input.node_id == 0)
		{
			return;
		}

		Node* in_node = nodes[input.node_id];
		in_node->Update(instance, deltaTime, data_stream);

		bool& node_result = *in_node->GetValue<bool>(instance, input.value_index);
		if (node_result)
		{
			result->True = true;
			result->False = false;
		}
		else
		{
			result->True = false;
			result->False = true;
		}
	}

	void* IfNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		Result* result = reinterpret_cast<Result*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &result->True;
			break;
		}
		case 1:
		{
			return &result->False;
			break;
		}
		}

		return nullptr;
	}

	void IfNode::Init(Graph* graph)
	{
		NodeInput input = {};
		input.node_id = 0;
		input.type = ValueType::Bool;
		input.value_index = INVALID_ID;

		m_inputs.push_back(input);

		NodeOutput out_1 = {};
		out_1.type = ValueType::Bool;
		out_1.value_index = 0;

		NodeOutput out_2 = {};
		out_2.type = ValueType::Bool;
		out_2.value_index = 1;

		m_outputs.push_back(out_1);
		m_outputs.push_back(out_2);
	}

	uint32_t Node::BeginNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream)
	{
		data_stream->Write(m_uuid);
		return data_stream->GetPosition();
	}

	void Node::EndNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream, uint32_t begin_pos)
	{
		uint32_t current_pos = data_stream->GetPosition();
		if (current_pos <= begin_pos)
		{
			data_stream->SetPosition( begin_pos - sizeof(m_uuid));
		}
		else
		{
			instance->IncrementNumNodes();
		}
	}

	// --------------------------------------------------------------------
}