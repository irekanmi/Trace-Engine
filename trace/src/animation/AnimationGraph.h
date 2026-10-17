#pragma once

#include "resource/Resource.h"
#include "scene/UUID.h"
#include "animation/AnimationNode.h"
#include "reflection/TypeRegistry.h"
#include "serialize/DataStream.h"
#include "resource/Ref.h"
#include "animation/Animation.h"
#include "node_system/GenericGraph.h"

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

	struct TransitionSet
	{
		Condition first;
		UUID second;
	};

	class Graph : public GenericGraph, public Resource
	{

	public:
		bool Create();
		//This is to prevent it Asset Manager(GenericAssetManager::CreateAssetHandle) to call the actual Create Function when using the reflection library
		bool Create(void* null_init);
		virtual void Destroy() override;


		virtual ~Graph() {};

		std::unordered_map<UUID, GenericNode*>& GetNodes() { return m_nodes; }
		PoseNode* GetRootNode() 
		{
			if (m_rootNode == 0)
			{
				return nullptr;
			}

			return (PoseNode*)m_nodes[m_rootNode];
		}
		virtual void CreateParameter(const std::string& param_name, GenericValueType type);
		
		void SetSkeleton(Ref<Skeleton> skeleton);
		Ref<Skeleton> GetSkeleton() { return m_skeleton; }


		std::vector<Ref<AnimationClip>>& GetAnimationDataSet() { return m_animationDataSet; }
		std::unordered_map<std::string, TransitionSet>& GetTransitionSet() { return m_parameterTranstions; }
		void AddAnimationClip(Ref<AnimationClip> clip);
		
	public:
		static Ref<Graph> Deserialize(UUID id);
		static Ref<Graph> Deserialize(DataStream* stream);

	private:
		Ref<Skeleton> m_skeleton;
		std::vector<Ref<AnimationClip>> m_animationDataSet;
		std::unordered_map<std::string, TransitionSet> m_parameterTranstions;
		

		ACCESS_CLASS_MEMBERS(Graph);
		GET_TYPE_ID;
	protected:

	};

	class GraphInstance : public GenericGraphInstance
	{

	public:
		GraphInstance();
		GraphInstance(GraphInstance& other);
		GraphInstance(const GraphInstance& other);
		virtual ~GraphInstance();

		//NOTE: To be called after creation[ After all data has been set]
		bool CreateInstance(Ref<Graph> graph, Scene* scene, UUID entity_id);
		virtual void DestroyInstance();

		void Start(Scene* scene, UUID id);
		void Stop(Scene* scene, UUID id);
		void Update(float deltaTime, Scene* scene, UUID id, Network::NetworkStream* data_stream = nullptr);
		bool HasStarted() { return m_started; }

		
		Ref<Graph> GetGraph() { return m_graph; }
		SkeletonInstance& GetSkeletonInstance() { return m_skeletonInstance; }

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
		virtual void set_parameter_data(const std::string& param_name, void* data, uint32_t size);

	private:
		Ref<Graph> m_graph;
		SkeletonInstance m_skeletonInstance;
		bool m_started = false;
		UUID m_entityHandle;


		//Networking ------------
		std::vector<bool> m_parameterDirty;
		uint32_t nodes_pos = 0;
		uint32_t num_nodes = 0;
		

	protected:
		ACCESS_CLASS_MEMBERS(GraphInstance);

	};

}