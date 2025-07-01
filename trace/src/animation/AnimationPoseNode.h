#pragma once

#include "animation/AnimationNode.h"
#include "animation/AnimationPose.h"
#include "core/Coretypes.h"
#include "motion_matching/MotionMatcher.h"
#include "motion_matching/Inertialize.h"

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

		GET_TYPE_ID;

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
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual void Destroy(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

		virtual void OnStateWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnStateRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;

		UUID CreateState(Graph* graph, StringID state_name);
		void Start(GraphInstance* instance);
		void SetCurrentNode(GraphInstance* instance, UUID uuid);

		UUID GetEntryNode() { return m_entryNode; }
		void SetEntryNode(UUID node_id) { m_entryNode = node_id; }

		std::vector<UUID>& GetStates() { return m_states; }
		void SetStates(std::vector<UUID>& states) { m_states = std::move(states); }

	private:
		std::vector<UUID> m_states;
		UUID m_entryNode = 0;


	protected:
		ACCESS_CLASS_MEMBERS(StateMachine);
		GET_TYPE_ID;

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
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual void Destroy(Graph* graph) override;
		void SetName(StringID name) { m_name = name; }
		StringID GetName() { return m_name; }
		virtual void Reset(GraphInstance* instance);
		void SetStateFlag(GraphInstance* instance, StateFlag flag);
		void SetStateMachine(UUID uuid) { m_stateMachine = uuid; }
		UUID GetStateMachine() { return m_stateMachine; }
		UUID CreateTransition(Graph* graph, UUID target_node);
		std::vector<UUID>& GetTransitions() { return m_transitions; }
		void SetTransitions(std::vector<UUID>& transitions) { m_transitions = std::move(transitions); }
		bool HasTransition(Graph* graph, UUID transition_node);
		void RemoveTransition(Graph* graph, UUID transition_node);


		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

	private:
		StringID m_name;
		std::vector<UUID> m_transitions;
		UUID m_stateMachine = 0;



	protected:
		ACCESS_CLASS_MEMBERS(StateNode);
		GET_TYPE_ID;
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
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

		virtual void OnStateWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnStateRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;

		bool ResolveConditions(GraphInstance* instance, float deltaTime);
		void SetDuration(float duration) { m_duration = duration; }
		void SetFromState(UUID uuid) { m_fromState = uuid; }
		void SetTargetState(UUID uuid) { m_targetState = uuid; }

		UUID GetFromState() { return m_fromState; }
		UUID GetTargetState() { return m_targetState; }
		float GetDuration() { return m_duration; }

	private:
		UUID m_fromState = 0;
		UUID m_targetState = 0;
		float m_duration = 0.0f;
		// float m_transtionWindowStart = 0.0f;
		// float m_transtionWindowEnd = 0.0f;
		// bool m_hasTransitionWindow = false;


	protected:

		ACCESS_CLASS_MEMBERS(TransitionNode);
		GET_TYPE_ID;
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
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;
		virtual void Reset(GraphInstance* instance);

		virtual void OnStateWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnStateRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;

		void SetAnimationClip(int32_t clip_index, Graph* graph);

		void SetLooping(bool loop) { m_looping = loop; }
		bool GetLooping() { return m_looping; }

		void SetAnimClipIndex(int32_t clip_index) { m_animClipIndex = clip_index; }
		int32_t GetAnimClipIndex() { return m_animClipIndex; }

		void SetDuration(float duration) { m_duration = duration; }
		float GetDuration() { return m_duration; }

	private:
		int32_t m_animClipIndex = -1;
		float m_duration = 0.0f;
		bool m_looping = false;

	protected:

		ACCESS_CLASS_MEMBERS(AnimationSampleNode);
		GET_TYPE_ID;
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
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;


	private:


	protected:
		GET_TYPE_ID;

	};

	class RetargetAnimationNode : public PoseNode
	{

	public:

		struct RuntimeData
		{
			float start_time = 0.0f;
			float elasped_time = 0.0f;
			PoseNodeResult final_pose;
			Pose animation_pose;
			Node::Definition definition;
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

		virtual void OnStateWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnStateRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;

		Ref<Skeleton> GetSkeleton() { return m_skeleton; }
		void SetSkeleton(Ref<Skeleton> skeleton) { m_skeleton = skeleton; }

		Ref<AnimationClip> GetAnimationClip() { return m_animation; }
		void SetAnimationClip(Ref<AnimationClip> animation) { m_animation = animation; }


	private:
		Ref<Skeleton> m_skeleton;
		Ref<AnimationClip> m_animation;

	protected:

		ACCESS_CLASS_MEMBERS(RetargetAnimationNode);
		GET_TYPE_ID;
	};

	class RetargetPoseNode : public PoseNode
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
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

		Ref<Skeleton> GetSkeleton() { return m_skeleton; }
		void SetSkeleton(Ref<Skeleton> skeleton) { m_skeleton = skeleton; }

	private:
		Ref<Skeleton> m_skeleton;

	protected:

		ACCESS_CLASS_MEMBERS(RetargetPoseNode);
		GET_TYPE_ID;
	};

	class WarpSection
	{

	public:

		int32_t start_frame;
		int32_t end_frame;

		glm::vec3 target;

	private:
	protected:

	};

	class WarpAnimationNode : public PoseNode
	{

	public:

		struct RuntimeData
		{
			float start_time = 0.0f;
			float elasped_time = 0.0f;
			PoseNodeResult final_pose;
			Node::Definition definition;
			std::vector<Transform> warped_root_motion;
			bool update_warp = true;
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;

		virtual void OnStateWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnStateRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkWrite_Server(GraphInstance* instance, Network::NetworkStream* data_stream) override;
		virtual void OnNetworkRead_Client(GraphInstance* instance, Network::NetworkStream* data_stream) override;

		Ref<AnimationClip> GetAnimationClip() { return m_animation; }
		void SetAnimationClip(Ref<AnimationClip> animation) { m_animation = animation; }


	private:
		bool get_root_motion_delta(GraphInstance* instance);
		bool UpdateWarp(GraphInstance* instance);

	private:
		Ref<AnimationClip> m_animation;
		std::vector<Transform> m_deltaTransforms;

	protected:

		ACCESS_CLASS_MEMBERS(WarpAnimationNode);
		GET_TYPE_ID;
	};


	class MotionMatchingNode : public PoseNode
	{

	public:

		struct RuntimeData
		{
			float start_time = 0.0f;
			float elasped_time = 0.0f;
			PoseNodeResult final_pose;
			Node::Definition definition;
			

			int32_t current_index = 0;
			float time_accummulator = 0.0f;
			AnimationClip* current_animation = nullptr;
			int32_t current_clip_index = 0;
			int32_t last_frame_index = 0;
			MotionMatching::Inertialize inertializer;
			Pose src_prev_pose;// Used for inertialization
			Pose trg_pose;// Used for inertialization
			Pose trg_prev_pose;// Used for inertialization
		};


	public:
		virtual bool Instanciate(GraphInstance* instance) override;
		virtual void Update(GraphInstance* instance, float deltaTime, Network::NetworkStream* data_stream = nullptr) override;
		virtual void* GetValueInternal(GraphInstance* instance, uint32_t value_index = 0) override;
		virtual void Init(Graph* graph) override;
		virtual PoseNodeResult* GetFinalPose(GraphInstance* instance) override;
		MotionMatching::MotionMatcher& GetMotionMatcher() { return m_matcher; }
		Ref<Skeleton> GetSkeleton() { return m_skeleton; }
		void SetSkeleton(Ref<Skeleton> skeleton) { m_skeleton = skeleton; }

	private:
		void FindNewPose(GraphInstance* instance, float deltaTime);

	private:
		MotionMatching::MotionMatcher m_matcher;
		Ref<Skeleton> m_skeleton;

	private:
		

	protected:

		ACCESS_CLASS_MEMBERS(MotionMatchingNode);
		GET_TYPE_ID;
	};

}
