#pragma once

#include "scene/UUID.h"
#include "core/Coretypes.h"
#include "core/Enums.h"

namespace trace::Animation {

	class GraphInstance;
	class Graph;

	enum class ValueType
	{
		Unknown,
		Pose,
		Float,
		Int,
		Bool
	};

	struct NodeInput
	{
		ValueType type = ValueType::Unknown;
		UUID node_id = 0;
		uint32_t value_index = INVALID_ID;
	};

	struct NodeOutput
	{
		ValueType type;
		uint32_t value_index;
	};

	class Node
	{

	public:

		virtual bool Instanciate(GraphInstance* instance) = 0;

		template<typename T>
		T* GetValue(GraphInstance* instance, uint32_t value_index = 0)
		{
			//.. Validate Type here

			return reinterpret_cast<T*>(GetValueInternal(instance, value_index));
		}

		virtual void Update(GraphInstance* instance, float deltaTime) = 0;
		virtual void Init(Graph* graph) = 0;

		// NOTE: Used to reset a node to it's start runtime values
		// Also reseting a node will reset all input nodes connected to it
		virtual void Reset(GraphInstance* instance) {}

		std::vector<NodeInput>& GetInputs() { return m_inputs; }
		std::vector<NodeOutput>& GetOutputs() { return m_outputs; }
		void SetUUID(UUID uuid) { m_uuid = uuid; }
		UUID GetUUID() { return m_uuid; }

	public:
		struct Definition
		{
			UpdateID update_id = 0;
			UUID node_id = 0;
		};

	protected:
		std::vector<NodeInput> m_inputs;
		std::vector<NodeOutput> m_outputs;
		UUID m_uuid = 0;

		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) = 0;

	};

	class EntryNode : public Node
	{
	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;

		void AddState(UUID node_id);

		struct Result
		{
			int32_t entry_node = 0;
		};

	private:
	protected:
	};

	class GetParameterNode : public Node
	{
	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;

		int32_t GetParameterIndex() { return m_parameterIndex; }
		void SetParameterIndex(int32_t index, Graph* graph);
		

	private:
		int32_t m_parameterIndex = -1;
	protected:
	};



}
