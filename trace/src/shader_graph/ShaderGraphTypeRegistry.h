#pragma once

#include "shader_graph/ShaderGraphNode.h"
#include "shader_graph/ShaderGraph.h"

#include "reflection/TypeRegistry.h"


namespace trace {

	BEGIN_REGISTER_CLASS(ShaderGraphNode)
		REGISTER_TYPE_PARENT(ShaderGraphNode, GenericNode);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(AddNode)
		REGISTER_TYPE_PARENT(AddNode, ShaderGraphNode);
		REGISTER_MEMBER(AddNode, m_type);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ConstantNode)
		REGISTER_TYPE_PARENT(ConstantNode, ShaderGraphNode);
		REGISTER_MEMBER(ConstantNode, m_type);
		REGISTER_MEMBER(ConstantNode, m_nodeValues);
		REGISTER_MEMBER(ConstantNode, m_nodeType);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(GetVariableNode)
		REGISTER_TYPE_PARENT(GetVariableNode, ShaderGraphNode);
		REGISTER_MEMBER(GetVariableNode, m_varIndex);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(FinalPBRNode)
		REGISTER_TYPE_PARENT(FinalPBRNode, ShaderGraphNode);
		REGISTER_MEMBER(FinalPBRNode, color);
		REGISTER_MEMBER(FinalPBRNode, occlusion);
		REGISTER_MEMBER(FinalPBRNode, metallic);
		REGISTER_MEMBER(FinalPBRNode, roughness);
		REGISTER_MEMBER(FinalPBRNode, emissionColor);
	END_REGISTER_CLASS;
	
	BEGIN_REGISTER_CLASS(ShaderGraph)
		REGISTER_TYPE_PARENT(ShaderGraph, GenericGraph);
		REGISTER_MEMBER(ShaderGraph, m_type);
		REGISTER_MEMBER(ShaderGraph, fragment_shader_root);
		REGISTER_MEMBER(ShaderGraph, m_cullMode);
	END_REGISTER_CLASS;

}
