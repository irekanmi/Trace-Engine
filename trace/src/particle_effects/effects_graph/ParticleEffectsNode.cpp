#include "pch.h"

#include "particle_effects/effects_graph/ParticleEffectsNode.h"
#include "particle_effects/ParticleGenerator.h"
#include "node_system/GenericGraph.h"
#include "core/Application.h"

namespace trace {




	void ParticleEffectNode::Init(GenericGraph* graph)
	{


		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Execute;
		input.value_index = INVALID_ID;

		m_inputs.resize(1);
		m_inputs[0] = input;

		GenericNodeOutput output = {};
		output.type = GenericValueType::Execute;
		output.value_index = 0;

		m_outputs.resize(1);
		m_outputs[0] = output;
		

	}

	void EffectsFinalNode::Init(GenericGraph* graph)
	{
		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Execute;
		input.value_index = INVALID_ID;

		m_inputs.resize(1);
		m_inputs[0] = input;
	}

	void EffectsFinalNode::Destroy(GenericGraph* graph)
	{
	}

	bool EffectsFinalNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		graph_instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void EffectsFinalNode::Update(int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();

		if (m_inputs[0].node_id == 0)
		{
			return;
		}

		ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[0].node_id);

		node->Update(particle_index, instance, deltaTime);


	}

	void EffectsRootNode::Init(GenericGraph* graph)
	{
		GenericNodeInput input = {};
		input.node_id = 0;
		input.type = GenericValueType::Execute;
		input.value_index = INVALID_ID;

		m_inputs.resize(1);
		m_inputs[0] = input;
	}

	void EffectsRootNode::Destroy(GenericGraph* graph)
	{
	}

	bool EffectsRootNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		return true;
	}

	void EffectsRootNode::Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime)
	{
	}

	void GenericEffectNode::Init(GenericGraph* graph)
	{
	}

	void GenericEffectNode::Destroy(GenericGraph* graph)
	{
	}

	bool GenericEffectNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		TRC_ASSERT(false, "Implement this funtion");
		return false;
	}

	void GenericEffectNode::Update(int32_t particle_index, GenericGraphInstance* instance, float deltaTime)
	{
		TRC_ASSERT(false, "Implement this funtion");
	}

	void GetParticleAttributeNode::Init(GenericGraph* graph)
	{
		ParticleEffectNode::Init(graph);

		GenericNodeOutput output = {};
		output.type = GenericValueType::Unknown;
		output.value_index = 0;

		m_outputs.resize(2);
		m_outputs[1] = output;

	}

	void GetParticleAttributeNode::Destroy(GenericGraph* graph)
	{
	}

	bool GetParticleAttributeNode::Instanciate(GenericGraphInstance* graph_instance)
	{
		graph_instance->GetNodesData()[m_uuid] = new NodeData;

		return true;
	}

	void GetParticleAttributeNode::Update(int32_t particle_index, GenericGraphInstance* graph_instance, float deltaTime)
	{
		ParticleGeneratorInstance* instance = (ParticleGeneratorInstance*)graph_instance;

		NodeData* info = (NodeData*)instance->GetNodesData()[m_uuid];

		if (info->frame_index == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		info->frame_index = Application::get_instance()->GetUpdateID();

		if (m_inputs[0].node_id == 0)
		{
			return;
		}

		ParticleEffectNode* node = (ParticleEffectNode*)instance->GetGenerator()->GetNode(m_inputs[0].node_id);

		node->Update(particle_index, instance, deltaTime);
	}

	void GetParticleAttributeNode::SetAttrID(UUID attr_id)
	{
	}

}