#pragma once

#include "AnimationState.h"
#include "resource/Resource.h"
#include "scene/UUID.h"
#include "animation/AnimationNode.h"
#include "reflection/TypeRegistry.h"

#include <vector>
#include <unordered_map>
#include <string>

namespace trace {
	class Scene;

	class AnimationGraph : public Resource
	{

	public:

		bool HasAnimationClip(Ref<AnimationClip> clip);

		std::vector<AnimationState>& GetStates() { return m_states; }
		int32_t GetCurrentStateIndex() { return m_currrentStateIndex; }
		int32_t GetStartIndex() { return m_startIndex; }
		AnimationState& GetCurrentState() { return m_states[m_currrentStateIndex]; }
		
		void SetAnimationStates(std::vector<AnimationState>& states) { m_states = std::move(states); }
		void SetCurrentStateIndex(int32_t current_index) { m_currrentStateIndex = current_index; }
		void SetStartIndex(int32_t start_index) { m_startIndex = start_index; }
		void SetAsRuntime();

		void Start();

		bool HasStarted() { return m_started; }

	private:
		int32_t m_currrentStateIndex = -1;
		int32_t m_startIndex = 0;
		std::vector<AnimationState> m_states;
		bool m_started = false;

	protected:

	};



}


namespace trace::Animation {

	class PoseNode;

	enum class ParameterType
	{
		Float,
		Bool,
		Int
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
		std::vector<Parameter>& GetParameters() { return m_parameters; }
		std::vector<Ref<AnimationClip>>& GetAnimationDataSet() { return m_animationDataSet; }
		std::unordered_map<std::string, TransitionSet>& GetTransitionSet() { return m_parameterTranstions; }
		void AddAnimationClip(Ref<AnimationClip> clip);
		
	public:
		static Ref<Graph> Deserialize(const std::string& file_path);

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
		void Update(float deltaTime);
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

	private:
		void set_parameter_data(const std::string& param_name, void* data, uint32_t size);

	private:
		Ref<Graph> m_graph;
		SkeletonInstance m_skeletonInstance;
		std::unordered_map<Node*, void*> m_nodesData;//NOTE: these member has high possibility to change when custom memory allocators has been added
		bool m_instanciated = false;
		bool m_started = false;

		// std::string: parameter name, uint32_t: index into parameter data index
		std::unordered_map<std::string, uint32_t> m_parameterLUT;
		std::vector<ParameterData> m_parameterData;
		

	protected:

	};

}