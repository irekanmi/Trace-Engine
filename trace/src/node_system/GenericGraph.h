#pragma once

#include "node_system/GenericNode.h"
#include "core/defines.h"

namespace trace {

	struct GenericParameterData
	{
		char data[GENERIC_PARAMETER_SIZE] = { 0 };
	};

	struct GenericParameter
	{
		std::string name;
		GenericValueType type = GenericValueType::Unknown;
	};

	struct GenericEdge
	{
		UUID from_node = 0;
		UUID to_node = 0;
	};

	class GenericGraph
	{

	public:


		virtual ~GenericGraph() {};
		virtual void DestroyGraph();

		std::unordered_map<UUID, GenericNode*>& GetNodes() { return m_nodes; }
		UUID GetRootNodeUUID() { return m_rootNode; }
		void SetRootNodeUUID(UUID node_id) { m_rootNode = node_id; }
		virtual void CreateParameter(const std::string& param_name, GenericValueType type) = 0;

		template<typename T>
		UUID CreateNode()
		{
			// Check if "T" is of type Node ...


			T* node = new T; // TODO: use custom allocator
			node->Init(this);
			UUID uuid = UUID::GenUUID();
			node->SetUUID(uuid);
			m_nodes[uuid] = node;

			return uuid;
		}


		GenericNode* GetNode(UUID node_id);
		void DestroyNode(UUID node_id);
		void DestroyNodeWithInputs(UUID node_id);
		std::vector<GenericParameter>& GetParameters() { return m_parameters; }

	public:

	private:
		std::unordered_map<UUID, GenericNode*> m_nodes;
		UUID m_rootNode = 0;
		std::vector<GenericParameter> m_parameters;


		ACCESS_CLASS_MEMBERS(GenericGraph);
		GET_TYPE_ID;
	protected:

	};

	class GenericGraphInstance
	{

	public:
		virtual ~GenericGraphInstance() {};

		virtual void DestroyInstance() = 0;
				
		template<typename T>
		void SetParameterData(const std::string& param_name, T& value)
		{
			set_parameter_data(param_name, &value, sizeof(T));
		}


		std::unordered_map<UUID, void*>& GetNodesData() { return m_nodesData; }
		std::unordered_map<std::string, uint32_t>& GetParametersLUT() { return m_parameterLUT; }
		std::vector<GenericParameterData>& GetParametersData() { return m_parameterData; }
		GenericParameterData* GetParameterData(GenericParameter& param);


	protected:
		virtual void set_parameter_data(const std::string& param_name, void* data, uint32_t size) = 0;

	protected:
		std::unordered_map<UUID, void*> m_nodesData;//NOTE: these member has high possibility to change when custom memory allocators has been added
		bool m_instanciated = false;

		// std::string: parameter name, uint32_t: index into parameter data index
		std::unordered_map<std::string, uint32_t> m_parameterLUT;
		std::vector<GenericParameterData> m_parameterData;


	protected:
		ACCESS_CLASS_MEMBERS(GenericGraphInstance);

	};

}
