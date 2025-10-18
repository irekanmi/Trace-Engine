#pragma once

#include "scene/UUID.h"
#include "core/Coretypes.h"
#include "core/Enums.h"
#include "reflection/TypeRegistry.h"

#define MAX_NODE_INPUTS 6

namespace trace {

	class GenericGraph;
	class GenericGraphInstance;

	enum class GenericValueType
	{
		Unknown,
		Bool,
		Float,
		Int,
		Vec2,
		Vec3,
		Vec4,
		Sampler2D,
		Execute,
		Max
	};

	struct GenericNodeInput
	{
		GenericValueType type = GenericValueType::Unknown;
		UUID node_id = 0;
		uint32_t value_index = INVALID_ID;
	};

	struct GenericNodeOutput
	{
		GenericValueType type;
		uint32_t value_index;
	};

	class GenericNode
	{

	public:

		virtual bool Instanciate(GenericGraphInstance* instance) = 0;

		template<typename T>
		T* GetValue(GenericGraphInstance* instance, uint32_t value_index = 0)
		{
			//.. Validate Type here

			return reinterpret_cast<T*>(GetValueInternal(instance, value_index));
		}

		virtual void Update(GenericGraphInstance* instance, float deltaTime) = 0;
		virtual void Run(GenericGraphInstance* instance) = 0;
		virtual void Init(GenericGraph* graph) = 0;
		virtual void Destroy(GenericGraph* graph) = 0;		

		std::vector<GenericNodeInput>& GetInputs() { return m_inputs; }
		std::vector<GenericNodeOutput>& GetOutputs() { return m_outputs; }
		void SetUUID(UUID uuid) { m_uuid = uuid; }
		UUID GetUUID() { return m_uuid; }

	public:
		struct Definition
		{
			UpdateID update_id = 0;
			UUID node_id = 0;
		};

	protected:
		std::vector<GenericNodeInput> m_inputs;
		std::vector<GenericNodeOutput> m_outputs;
		UUID m_uuid = 0;

		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) = 0;

		ACCESS_CLASS_MEMBERS(GenericNode);
		GET_TYPE_ID;
	};
}
