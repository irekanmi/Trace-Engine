#pragma once

#include "node_system/GenericNode.h"
#include "node_system/GenericGraph.h"
#include "reflection/TypeRegistry.h"

namespace trace {

	
	REGISTER_TYPE(GenericValueType);

	BEGIN_REGISTER_CLASS(GenericParameterData)
		REGISTER_TYPE(GenericParameterData);
		REGISTER_MEMBER(GenericParameterData, data);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GenericParameter)
		REGISTER_TYPE(GenericParameter);
		REGISTER_MEMBER(GenericParameter, name);
		REGISTER_MEMBER(GenericParameter, type);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GenericNodeInput)
		REGISTER_TYPE(GenericNodeInput);
		REGISTER_MEMBER(GenericNodeInput, type);
		REGISTER_MEMBER(GenericNodeInput, node_id);
		REGISTER_MEMBER(GenericNodeInput, value_index);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GenericNodeOutput)
		REGISTER_TYPE(GenericNodeOutput);
		REGISTER_MEMBER(GenericNodeOutput, type);
		REGISTER_MEMBER(GenericNodeOutput, value_index);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GenericNode)
		REGISTER_TYPE(GenericNode);
		REGISTER_MEMBER(GenericNode, m_inputs);
		REGISTER_MEMBER(GenericNode, m_outputs);
		REGISTER_MEMBER(GenericNode, m_uuid);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GenericGraph)
		REGISTER_TYPE(GenericGraph);
		REGISTER_MEMBER(GenericGraph, m_nodes);
		REGISTER_MEMBER(GenericGraph, m_rootNode);
		REGISTER_MEMBER(GenericGraph, m_parameters);
	END_REGISTER_CLASS;

}