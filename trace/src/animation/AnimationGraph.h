#pragma once

#include "resource/Resource.h"
#include "scene/UUID.h"
#include "animation/AnimationNode.h"
#include "reflection/TypeRegistry.h"
#include "serialize/DataStream.h"
#include "resource/Ref.h"
#include "animation/Animation.h"

#include <vector>
#include <unordered_map>
#include <string>

namespace trace {
	class Scene;
}

namespace trace::Network {

	class NetworkStream;
}

namespace trace::Animation {

	class PoseNode;

	enum class ParameterType
	{
		Float,
		Bool,
		Int,
		Max
	};

	struct ParameterData
	{
		char data[16] = {0};
	};

	//using Parameter = std::pair<std::string, ParameterType>;
	struct Parameter
	{
		std::string first;
		ParameterType second;
	};

	enum class ConditionType
	{
		GreaterThan,
		GreaterThanOrEquals,
		LessThan,
		LessThanOrEquals,
		Equals
	};

	struct ConditionData
	{
		char data[16] = { 0 };
	};

	struct Condition
	{
		ConditionType type;
		ConditionData data;
	};

	//using TransitionSet = std::pair<Condition, UUID>;// first: condition, second: Transiiton 
	struct TransitionSet
	{
		Condition first;
		UUID second;
	};

	class Graph : public Resource
	{

	public:
		bool Create();
		virtual void Destroy() override;

		std::unordered_map<UUID, Node*>& GetNodes() { return m_nodes; }
		PoseNode* GetRootNode() 
		{
			if (m_rootNode == 0)
			{
				return nullptr;
			}

			return (PoseNode*)m_nodes[m_rootNode];
		}
		UUID GetRootNodeUUID() { return m_rootNode; }
		void SetRootNodeUUID(UUID node_id) { m_rootNode = node_id; }
		void CreateParameter(const std::string& param_name, ParameterType type);
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

		void SetSkeleton(Ref<Skeleton> skeleton);
		Ref<Skeleton> GetSkeleton() { return m_skeleton; }


		Node* GetNode(UUID node_id);
		void DestroyNode(UUID node_id);
		void DestroyNodeWithInputs(UUID node_id);
		std::vector<Parameter>& GetParameters() { return m_parameters; }
		std::vector<Ref<AnimationClip>>& GetAnimationDataSet() { return m_animationDataSet; }
		std::unordered_map<std::string, TransitionSet>& GetTransitionSet() { return m_parameterTranstions; }
		void AddAnimationClip(Ref<AnimationClip> clip);
		
	public:
		static Ref<Graph> Deserialize(UUID id);
		static Ref<Graph> Deserialize(DataStream* stream);

	private:
		std::unordered_map<UUID, Node*> m_nodes;
		UUID m_rootNode = 0;
		Ref<Skeleton> m_skeleton;
		std::vector<Ref<AnimationClip>> m_animationDataSet;
		std::vector<Parameter> m_parameters;
		std::unordered_map<std::string, TransitionSet> m_parameterTranstions;
		

		ACCESS_CLASS_MEMBERS(Graph);
		GET_TYPE_ID;
	protected:

	};

	class GraphInstance
	{

	public:
		GraphInstance();
		GraphInstance(GraphInstance& other);
		GraphInstance(const GraphInstance& other);
		~GraphInstance();

		//NOTE: To be called after creation[ After all data has been set]
		bool CreateInstance(Ref<Graph> graph, Scene* scene, UUID entity_id);
		void DestroyInstance();

		void Start(Scene* scene, UUID id);
		void Stop(Scene* scene, UUID id);
		void Update(float deltaTime, Scene* scene, UUID id, Network::NetworkStream* data_stream = nullptr);
		bool HasStarted() { return m_started; }

		template<typename T>
		void SetParameterData(const std::string& param_name, T& value)
		{
			set_parameter_data(param_name, &value, sizeof(T));
		}
		

		std::unordered_map<Node*, void*>& GetNodesData() { return m_nodesData; }
		Ref<Graph> GetGraph() { return m_graph; }
		SkeletonInstance& GetSkeletonInstance() { return m_skeletonInstance; }
		std::unordered_map<std::string, uint32_t>& GetParametersLUT() { return m_parameterLUT; }
		std::vector<ParameterData>& GetParametersData() { return m_parameterData; }
		ParameterData* GetParameterData(Parameter& param);

		UUID GetEntityHandle() { return m_entityHandle; }
		void SetEntityHandle(UUID entity_handle) { m_entityHandle = entity_handle; }

		//Networking ----------
		void OnStateWrite_Server(Network::NetworkStream* data_stream);
		void OnStateWrite_Client(Network::NetworkStream* data_stream);
		void OnStateRead_Client(Network::NetworkStream* data_stream);
		void OnStateRead_Server(Network::NetworkStream* data_stream);
		void BeginNetworkWrite_Server(Network::NetworkStream* data_stream);
		void BeginNetworkWrite_Client(Network::NetworkStream* data_stream);
		void EndNetworkWrite_Server(Network::NetworkStream* data_stream);
		void EndNetworkWrite_Client(Network::NetworkStream* data_stream);
		void OnNetworkRead_Client(Network::NetworkStream* data_stream, bool accept_packet);
		void OnNetworkRead_Server(Network::NetworkStream* data_stream);
		void IncrementNumNodes() { ++num_nodes; }

	private:
		void set_parameter_data(const std::string& param_name, void* data, uint32_t size);

	private:
		Ref<Graph> m_graph;
		SkeletonInstance m_skeletonInstance;
		std::unordered_map<Node*, void*> m_nodesData;//NOTE: these member has high possibility to change when custom memory allocators has been added
		bool m_instanciated = false;
		bool m_started = false;
		UUID m_entityHandle;

		// std::string: parameter name, uint32_t: index into parameter data index
		std::unordered_map<std::string, uint32_t> m_parameterLUT;
		std::vector<ParameterData> m_parameterData;

		//Networking ------------
		std::vector<bool> m_parameterDirty;
		uint32_t nodes_pos = 0;
		uint32_t num_nodes = 0;
		

	protected:
		ACCESS_CLASS_MEMBERS(GraphInstance);

	};

}