#pragma once

#include "animation/AnimationNode.h"
#include "animation/AnimationPose.h"
#include "core/Coretypes.h"

namespace trace::Animation {

	struct PoseNodeResult
	{
		Pose pose_data;
		float clip_duration = 0.0f;
		float current_time = 0.0f;
		float prev_time = 0.0f;
	};

	class PoseNode : public Node
	{

	public:


		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) = 0;

	private:
	protected:

	};

	class StateNode;
	class StateMachine : public PoseNode
	{

	public:

		struct RuntimeData
		{
			PoseNode* current_node = nullptr;
			Node::Definition definition;
			PoseNodeResult* pose_result = nullptr;
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;
		
		UUID CreateState(Graph* graph, StringID state_name);
		void Start(GraphInstance* instance);
		void SetCurrentNode(GraphInstance* instance, UUID uuid);

	private:
		std::vector<UUID> m_states;
		UUID m_entryNode = 0;


	protected:

	};

	enum class StateFlag
	{
		Running,
		InTransition
	};

	class TransitionNode;
	class StateNode : public PoseNode
	{

	public:

		struct RuntimeData
		{
			PoseNodeResult* final_pose = nullptr;
			Node::Definition definition;
			float elasped_time = 0.0f;
			StateFlag current_flag = StateFlag::Running;
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		void SetName(StringID name) { m_name = name; }
		StringID GetName() { return m_name; }
		virtual void Reset(GraphInstance* instance);
		void SetStateFlag(GraphInstance* instance, StateFlag flag);
		void SetStateMachine(UUID uuid) { m_stateMachine = uuid; }
		UUID GetStateMachine() { return m_stateMachine; }
		UUID CreateTransition(Graph* graph, UUID target_node);

		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

	private:
		StringID m_name = 0;
		std::vector<UUID> m_transitions;
		UUID m_stateMachine = 0;
		


	protected:

	};

	class TransitionNode : public PoseNode
	{

	public:

		struct RuntimeData
		{
			PoseNodeResult pose_result;
			Node::Definition definition;
			float elapsed_time = 0.0f;
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

		bool ResolveConditions(GraphInstance* instance, float deltaTime);
		void SetDuration(float duration) { m_duration = duration; }
		void SetFromState(UUID uuid) { m_fromState = uuid; }
		void SetTargetState(UUID uuid) { m_targetState = uuid; }

	private:
		UUID m_fromState = 0;
		UUID m_targetState = 0;
		float m_duration = 0.0f;
		// float m_transtionWindowStart = 0.0f;
		// float m_transtionWindowEnd = 0.0f;
		// bool m_hasTransitionWindow = false;


	protected:

	};

	class AnimationSampleNode : public PoseNode
	{

	public:

		struct RuntimeData
		{
			float start_time = 0.0f;
			float elasped_time = 0.0f;
			PoseNodeResult final_pose;
			Node::Definition definition;
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;
		virtual void Reset(GraphInstance* instance);

		void SetAnimationClip(int32_t clip_index, Graph* graph);
		void SetLooping(bool loop) { m_looping = loop; }

	private:
		int m_animClipIndex = -1;
		float m_duration = 0.0f;
		bool m_looping = false;

	protected:

	};

	class FinalOutputNode : public PoseNode
	{

	public:
		struct RuntimeData
		{
			PoseNodeResult* pose_result = nullptr;
			Node::Definition definition;
			float elasped_time = 0.0f;
		};
		
	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;


	private:


	protected:

	};
}
