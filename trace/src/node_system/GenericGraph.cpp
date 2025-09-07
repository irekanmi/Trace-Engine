#include "pch.h"

#include "node_system/GenericGraph.h"
#include "core/io/Logging.h"

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

}

