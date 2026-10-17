#pragma once

#include "scene/UUID.h"
#include "core/Coretypes.h"
#include "core/Enums.h"
#include "node_system/GenericNode.h"
#include "reflection/TypeRegistry.h"

namespace trace::Network {

	class NetworkStream;
}

namespace trace::Animation {

	class GraphInstance;
	class Graph;


	class Node : public GenericNode
	{

	public:

		virtual bool Instanciate(GenericGraphInstance* instance) { return false; };

		template<typename T>
		T* GetValue(GraphInstance* instance, uint32_t value_index = 0)
		{
			//.. Validate Type here

			return reinterpret_cast<T*>(GetValueInternal(instance, value_index));
		}

		virtual void Update(GenericGraphInstance* instance, float deltaTime) override {};
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) {};
		virtual void Run(GenericGraphInstance* instance) {};
		virtual void Init(GenericGraph* graph) {};
		virtual void Destroy(GenericGraph* graph) {};
		
		// Networking ------------------------------
		virtual void OnStateWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) {};
		virtual void OnStateRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) {};
		virtual uint32_t BeginNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream);
		virtual void EndNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream, uint32_t begin_pos);
		virtual void OnNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) {};
		virtual void OnNetworkRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream, bool accept_packet = true) {};

		// NOTE: Used to reset a node to it's start runtime values
		// Also reseting a node will reset all input nodes connected to it
		virtual void Reset(GraphInstance* instance) {}
		virtual size_t GetHash() { return typeid(*this).hash_code(); }
		void SetUUID(UUID uuid) { m_uuid = uuid; }
		UUID GetUUID() { return m_uuid; }

	public:

	protected:

		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) { return nullptr; };

		ACCESS_CLASS_MEMBERS(Node);
		GET_TYPE_ID;
	};

	class EntryNode : public Node
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(GenericGraph* graph) override;


		struct Result
		{
			UUID entry_node = 0;
		};

	private:
	protected:

		GET_TYPE_ID;
	};

	class GetParameterNode : public Node
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(GenericGraph* graph) override;

		int32_t GetParameterIndex() { return m_parameterIndex; }
		void SetParameterIndex(int32_t index, Graph* graph);


	private:
		int32_t m_parameterIndex = -1;
	protected:

		ACCESS_CLASS_MEMBERS(GetParameterNode);
		GET_TYPE_ID;
	};

	class IfNode : public Node
	{
	public:
		virtual bool Instanciate(GenericGraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GenericGraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(GenericGraph* graph) override;

		struct Result
		{
			bool True = false;
			bool False = false;
		};

	private:
	protected:

		ACCESS_CLASS_MEMBERS(IfNode);
		GET_TYPE_ID;
	};



}
