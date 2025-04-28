#include "pch.h"

#include "animation/AnimationPoseNode.h"
#include "core/Application.h"
#include "animation/AnimationEngine.h"
#include "scene/Entity.h"
#include "animation/AnimationBlend.h"
#include "animation/AnimationGraph.h"
#include "debug/Debugger.h"


namespace trace::Animation {



	bool StateNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		return true;
	}

	void StateNode::Update(GraphInstance* instance, float deltaTime)
	{

		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		data->definition.update_id = Application::get_instance()->GetUpdateID();

		StateMachine* state_machine = (StateMachine*)nodes[m_stateMachine];

		if (data->current_flag == StateFlag::Running)
		{

			for (UUID& transition_uuid : m_transitions)
			{
				TransitionNode* transition = (TransitionNode*)nodes[transition_uuid];

				if (transition->ResolveConditions(instance, deltaTime))
				{
					state_machine->SetCurrentNode(instance, transition_uuid);
					transition->Update(instance, deltaTime);
					data->current_flag = StateFlag::InTransition;
					return;
				}

			}
		}

		NodeInput& input = m_inputs[0];
		if (input.node_id == 0)
		{
			return;
		}

		Node* in_node = nodes[input.node_id];
		in_node->Update(instance, deltaTime);
		data->final_pose = in_node->GetValue<PoseNodeResult>(instance, input.value_index);


		
		
	}

	void* StateNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		void* result = nullptr;

		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);
		
		switch (value_index)
		{
		case 0:
		{
			result = data->final_pose;
			break;
		}
		}
		
		return result;
	}

	void StateNode::Init(Graph* graph)
	{
		NodeInput in_pose = {};
		in_pose.node_id = 0;
		in_pose.type = ValueType::Pose;
		in_pose.value_index = INVALID_ID;

		m_inputs.push_back(in_pose);

		NodeOutput out_pose = {};
		out_pose.type = ValueType::Pose;
		out_pose.value_index = 0;

		m_outputs.push_back(out_pose);
	}

	void StateNode::Destroy(Graph* graph)
	{
		for (UUID& transition : m_transitions)
		{
			RemoveTransition(graph, transition);
		}
	}

	void StateNode::Reset(GraphInstance* instance)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);
		data->elasped_time = 0.0f;
	

		for (NodeInput& input : m_inputs)
		{
			if (input.node_id == 0)
			{
				continue;
			}

			Node* in_node = nodes[input.node_id];
			in_node->Reset(instance);
		}
	}

	void StateNode::SetStateFlag(GraphInstance* instance, StateFlag flag)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);
		data->current_flag = flag;
	}

			
			
	UUID StateNode::CreateTransition(Graph* graph, UUID target_node)
	{
		UUID transition_id = graph->CreateNode<TransitionNode>();
		TransitionNode* transition_node = (TransitionNode*)graph->GetNode(transition_id);
		transition_node->SetFromState(m_uuid);
		transition_node->SetTargetState(target_node);
		transition_node->SetDuration(0.25f);

		m_transitions.push_back(transition_id);

		return transition_id;
	}

	bool StateNode::HasTransition(Graph* graph, UUID transition_node)
	{
		auto it = std::find(m_transitions.begin(), m_transitions.end(), transition_node);
		return it != m_transitions.end();
	}

	void StateNode::RemoveTransition(Graph* graph, UUID transition_node)
	{
		if (HasTransition(graph, transition_node))
		{
			int32_t index = 0;
			for (UUID& id : m_transitions)
			{
				if (id == transition_node)
				{
					graph->DestroyNodeWithInputs(id);
					break;
				}
				index++;
			}
			m_transitions[index] = m_transitions.back();
			m_transitions.pop_back();
		}
	}

	PoseNodeResult* StateNode::GetFinalPose(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (result->final_pose != nullptr)
		{
			return result->final_pose;
		}

		return nullptr;
	}


	// Animation Sample Node -----------------------------------------------------------

	bool AnimationSampleNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		result->final_pose.pose_data.Init(&instance->GetSkeletonInstance());

		return true;
	}

	void AnimationSampleNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}		


		std::vector<Ref<AnimationClip>>& animations = instance->GetGraph()->GetAnimationDataSet();

		//Check if animation index is valid
		if (m_animClipIndex < 0 || (m_animClipIndex > animations.size() - 1))
		{
			return;
		}

		if (data->elasped_time == 0.0f)
		{
			data->start_time = Application::get_instance()->GetClock().GetElapsedTime();
		}
		
		Ref<AnimationClip>& clip = animations[m_animClipIndex];

		float next_frame_time = data->elasped_time + deltaTime;
		if (clip->HasRootMotion())
		{
			AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(clip, data->elasped_time, next_frame_time, &data->final_pose.pose_data, m_looping);
		}
		else
		{
			AnimationEngine::get_instance()->SampleClip(clip, data->elasped_time, &data->final_pose.pose_data, m_looping);
			Transform& root_motion_delta = data->final_pose.pose_data.GetRootMotionDelta();
			root_motion_delta = Transform::Identity();
		}

		data->elasped_time = next_frame_time;
		data->definition.update_id = Application::get_instance()->GetUpdateID();

	}

	void* AnimationSampleNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &data->final_pose;
			break;
		}
		}

		return nullptr;
	}

	void AnimationSampleNode::Init(Graph* graph)
	{
		NodeOutput output = {};
		output.type = ValueType::Pose;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	PoseNodeResult* AnimationSampleNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}
	
	void AnimationSampleNode::Reset(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		data->elasped_time = 0.0f;
		data->start_time = 0.0f;

	}

	void AnimationSampleNode::SetAnimationClip(int32_t clip_index, Graph* graph)
	{
		std::vector<Ref<AnimationClip>> animations = graph->GetAnimationDataSet();

		if (clip_index < 0 || clip_index > animations.size() - 1)
		{
			return;
		}

		m_animClipIndex = clip_index;

		Ref<AnimationClip>& clip = animations[clip_index];
		m_duration = clip->GetDuration();


	}

	// ---------------------------------------------------------------------------------------------

	// Transition Node -----------------------------------------------------------------------------

	bool TransitionNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		result->pose_result.pose_data.Init(&instance->GetSkeletonInstance());

		return true;
	}

	void TransitionNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		data->definition.update_id = Application::get_instance()->GetUpdateID();


		StateNode* current_state = (StateNode*)nodes[m_fromState];
		StateNode* target_state = (StateNode*)nodes[m_targetState];

		if (data->elapsed_time == 0.0f)
		{
			target_state->SetStateFlag(instance, StateFlag::InTransition);
		}
		StateMachine* state_machine = (StateMachine*)nodes[current_state->GetStateMachine()];

		float blend_weight = data->elapsed_time / m_duration;
		current_state->Update(instance, deltaTime);
		PoseNodeResult* current_state_result = current_state->GetFinalPose(instance);

		target_state->Update(instance, deltaTime);
		PoseNodeResult* target_state_result = target_state->GetFinalPose(instance);

		

		if (data->elapsed_time >= m_duration)
		{
			current_state->Reset(instance);
			data->elapsed_time = 0.0f;
			
			target_state->SetStateFlag(instance, StateFlag::Running);
			state_machine->SetCurrentNode(instance, target_state->GetUUID());

			return;
		}
		
		if (!current_state_result && target_state_result)
		{
			BlendPose(&target_state_result->pose_data, &target_state_result->pose_data, &data->pose_result.pose_data, blend_weight);
		}

		if (current_state_result && !target_state_result)
		{
			BlendPose(&current_state_result->pose_data, &current_state_result->pose_data, &data->pose_result.pose_data, blend_weight);
		}

		if (current_state_result && target_state_result)
		{
			BlendPose(&current_state_result->pose_data, &target_state_result->pose_data, &data->pose_result.pose_data, blend_weight);
		}

		data->elapsed_time += deltaTime;
		
	}

	void* TransitionNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{

		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &data->pose_result;
			break;
		}
		}

		return nullptr;

		return nullptr;
	}

	void TransitionNode::Init(Graph* graph)
	{
		NodeInput condition_input = {};
		condition_input.node_id = 0;
		condition_input.type = ValueType::Bool;
		condition_input.value_index = INVALID_ID;

		m_inputs.push_back(condition_input);
	}

	PoseNodeResult* TransitionNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}

	bool TransitionNode::ResolveConditions(GraphInstance* instance, float deltaTime)
	{

		NodeInput& input = m_inputs[0];

		if (input.node_id == 0)
		{
			return false;
		}

		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();

		Node* node = nodes[input.node_id];
		node->Update(instance, deltaTime);
		
		bool* result = node->GetValue<bool>(instance, input.value_index);
		TRC_ASSERT(result != nullptr, "Function: {}", __FUNCTION__);

		return *result;
	}

	// -------------------------------------------------------------------------------------

	// State Machine Node ---------------------------------------------------------------

	bool StateMachine::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		return true;
	}

	void StateMachine::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		data->definition.update_id = Application::get_instance()->GetUpdateID();

		if (data->current_node == nullptr)
		{
			Start(instance);
		}

		TRC_ASSERT(data->current_node != nullptr, "Function: {}", __FUNCTION__);

		data->current_node->Update(instance, deltaTime);
		data->pose_result = data->current_node->GetFinalPose(instance);

		
	}

	void* StateMachine::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		void* result = nullptr;

		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			result = data->pose_result;
			break;
		}
		}

		return result;
	}

	void StateMachine::Init(Graph* graph)
	{
		NodeOutput output = {};
		output.type = ValueType::Pose;
		output.value_index = 0;

		m_outputs.push_back(output);

		m_entryNode = graph->CreateNode<EntryNode>();

	}

	void StateMachine::Destroy(Graph* graph)
	{
		for (UUID& state : m_states)
		{
			graph->DestroyNode(state);
		}
	}

	PoseNodeResult* StateMachine::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}

	UUID StateMachine::CreateState(Graph* graph, StringID state_name)
	{
		UUID state_id = graph->CreateNode<StateNode>();
		m_states.push_back(state_id);

		StateNode* state_node = (StateNode*)graph->GetNode(state_id);
		state_node->SetName(state_name);
		state_node->SetStateMachine(m_uuid);

		return state_id;
	}

	void StateMachine::Start(GraphInstance* instance)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);
		EntryNode* entry_node = (EntryNode*)nodes[m_entryNode];

		entry_node->Update(instance, 0.0f);
		UUID* entry_state_index = entry_node->GetValue<UUID>(instance);

		if (*entry_state_index != 0)
		{
			data->current_node = (PoseNode*)nodes[*entry_state_index];
		}
		else
		{
			data->current_node = (PoseNode*)nodes[m_states[0]];
		}


	}

	void StateMachine::SetCurrentNode(GraphInstance* instance, UUID uuid)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);
		data->current_node = (PoseNode*)nodes[uuid];

	}

	// ------------------------------------------------------------------------------------

	
	// Final Output Node ------------------------------------------------------------

	bool FinalOutputNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		return true;
	}

	void FinalOutputNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}
		data->definition.update_id = Application::get_instance()->GetUpdateID();

		NodeInput& input = m_inputs[0];
		if (input.node_id == 0)
		{
			return;
		}

		Node* in_node = nodes[input.node_id];
		in_node->Update(instance, deltaTime);
		data->pose_result = in_node->GetValue<PoseNodeResult>(instance, input.value_index);
	}

	void* FinalOutputNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		void* result = nullptr;

		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			result = data->pose_result;
			break;
		}
		}

		return result;
	}

	void FinalOutputNode::Init(Graph* graph)
	{
		NodeInput in_pose = {};
		in_pose.node_id = 0;
		in_pose.type = ValueType::Pose;
		in_pose.value_index = INVALID_ID;

		m_inputs.push_back(in_pose);
	}

	PoseNodeResult* FinalOutputNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}
	
	// ----------------------------------------------------------------------------


	// Retarget Animation Node -----------------------------------------------------------

	bool RetargetAnimationNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		result->final_pose.pose_data.Init(&instance->GetSkeletonInstance());
		result->animation_pose.Init(m_skeleton);

		return true;
	}

	void RetargetAnimationNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (!m_animation)
		{
			return;
		}

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}


		std::vector<Ref<AnimationClip>>& animations = instance->GetGraph()->GetAnimationDataSet();

		

		if (data->elasped_time == 0.0f)
		{
			data->start_time = Application::get_instance()->GetClock().GetElapsedTime();
		}

		Ref<AnimationClip> clip = m_animation;
		Animation::Pose& clip_pose = data->animation_pose;
		

		float next_frame_time = data->elasped_time + deltaTime;
		if (clip->HasRootMotion())
		{
			AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(clip, m_skeleton, data->elasped_time, next_frame_time, &clip_pose, true);
		}
		else
		{
			AnimationEngine::get_instance()->SampleClip(clip, m_skeleton, data->elasped_time, &clip_pose, true);
			Transform& root_motion_delta = clip_pose.GetRootMotionDelta();
			root_motion_delta = Transform::Identity();
		}

		

		Ref<Animation::Skeleton> skeleton = instance->GetGraph()->GetSkeleton();
		Ref<HumanoidRig> target_rig = skeleton->GetHumanoidRig();
		Ref<HumanoidRig> source_rig = m_skeleton->GetHumanoidRig();

		// Initialize Final Pose with the bind pose
		skeleton->GetBindPose(data->final_pose.pose_data.GetLocalPose());

		if (target_rig && source_rig)
		{

			auto& target_rig_bones = target_rig->GetHumanoidBones();
			auto& source_rig_bones = source_rig->GetHumanoidBones();
			
			int32_t index = 0;
			for (int32_t& i : source_rig_bones)
			{
				if (i < 0)
				{
					++index;
					continue;
				}

				if (target_rig_bones[index] < 0)
				{
					++index;
					continue;
				}

				int32_t source_bone_index = i;
				int32_t target_bone_index = target_rig_bones[index];

				Animation::Bone& source_bone = *m_skeleton->GetBone(source_bone_index);
				Animation::Bone& target_bone = *skeleton->GetBone(target_bone_index);

				glm::mat4 target_bind_pose = skeleton->GetBoneGlobalBindPose(target_bone_index);
				//glm::mat4 inv_target_bind_pose = glm::inverse(target_bind_pose);

				glm::mat4 source_bind_pose = m_skeleton->GetBoneGlobalBindPose(source_bone_index);
				glm::mat4 inv_source_bind_pose = glm::inverse(source_bind_pose);

				glm::mat4 offset = inv_source_bind_pose * target_bind_pose;

				glm::mat4 animation_pose = clip_pose.GetBoneGlobalPose(m_skeleton, source_bone_index);
				Transform debug_pose(animation_pose);
				debug_pose.SetPosition( debug_pose.GetPosition() * 0.1f);
				Debugger::get_instance()->DrawDebugSphere(0.8f, 4, debug_pose.GetLocalMatrix(), TRC_COL32(0, 255, 0, 255));

				//glm::mat4 pose_result = offset * animation_pose;
				glm::mat4 pose_result = animation_pose * offset;

				if (target_bone.GetParentIndex() != -1)
				{
					glm::mat4 target_parent_pose = data->final_pose.pose_data.GetBoneGlobalPose(target_bone.GetParentIndex());
					pose_result = glm::inverse(target_parent_pose) * pose_result;
				}

				std::vector<Transform>& target_local_pose = data->final_pose.pose_data.GetLocalPose();
				target_local_pose[target_bone_index] = Transform(pose_result);

				++index;
			}

			if (m_animation->HasRootMotion())
			{
				Transform& src_root_motion_delta = clip_pose.GetRootMotionDelta();
				Transform& trg_root_motion_delta = data->final_pose.pose_data.GetRootMotionDelta();
				trg_root_motion_delta = src_root_motion_delta;


			}
		}

		data->elasped_time = next_frame_time;
		data->definition.update_id = Application::get_instance()->GetUpdateID();

	}

	void* RetargetAnimationNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &data->final_pose;
			break;
		}
		}

		return nullptr;
	}

	void RetargetAnimationNode::Init(Graph* graph)
	{
		NodeOutput output = {};
		output.type = ValueType::Pose;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	PoseNodeResult* RetargetAnimationNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}
	
	// -----------------------------------------------------------------------------------

	// Retarget Pose Node -----------------------------------------------------------

	bool RetargetPoseNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		result->final_pose.pose_data.Init(&instance->GetSkeletonInstance());

		return true;
	}

	void RetargetPoseNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);


		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}

		NodeInput& input = m_inputs[0];
		if (input.node_id == 0)
		{
			return;
		}

		if (!m_skeleton)
		{
			return;
		}

		Node* in_node = nodes[input.node_id];
		in_node->Update(instance, deltaTime);


		if (data->elasped_time == 0.0f)
		{
			data->start_time = Application::get_instance()->GetClock().GetElapsedTime();
		}

		Animation::Pose& clip_pose = in_node->GetValue<PoseNodeResult>(instance, input.value_index)->pose_data;


		



		Ref<Animation::Skeleton> skeleton = instance->GetGraph()->GetSkeleton();
		Ref<HumanoidRig> target_rig = skeleton->GetHumanoidRig();
		Ref<HumanoidRig> source_rig = m_skeleton->GetHumanoidRig();

		// Initialize Final Pose with the bind pose
		skeleton->GetBindPose(data->final_pose.pose_data.GetLocalPose());

		if (target_rig && source_rig)
		{

			auto& target_rig_bones = target_rig->GetHumanoidBones();
			auto& source_rig_bones = source_rig->GetHumanoidBones();

			int32_t index = 0;
			for (int32_t& i : source_rig_bones)
			{
				if (i < 0)
				{
					++index;
					continue;
				}

				if (target_rig_bones[index] < 0)
				{
					++index;
					continue;
				}

				int32_t source_bone_index = i;
				int32_t target_bone_index = target_rig_bones[index];

				Animation::Bone& source_bone = *m_skeleton->GetBone(source_bone_index);
				Animation::Bone& target_bone = *skeleton->GetBone(target_bone_index);

				glm::mat4 target_bind_pose = skeleton->GetBoneGlobalBindPose(target_bone_index);
				//glm::mat4 inv_target_bind_pose = glm::inverse(target_bind_pose);

				glm::mat4 source_bind_pose = m_skeleton->GetBoneGlobalBindPose(source_bone_index);
				glm::mat4 inv_source_bind_pose = glm::inverse(source_bind_pose);

				glm::mat4 offset = inv_source_bind_pose * target_bind_pose;

				glm::mat4 animation_pose = clip_pose.GetBoneGlobalPose(m_skeleton, source_bone_index);
				
				//glm::mat4 pose_result = offset * animation_pose;
				glm::mat4 pose_result = animation_pose * offset;

				if (target_bone.GetParentIndex() != -1)
				{
					glm::mat4 target_parent_pose = data->final_pose.pose_data.GetBoneGlobalPose(target_bone.GetParentIndex());
					pose_result = glm::inverse(target_parent_pose) * pose_result;
				}

				std::vector<Transform>& target_local_pose = data->final_pose.pose_data.GetLocalPose();
				target_local_pose[target_bone_index] = Transform(pose_result);

				++index;
			}

			Transform& src_root_motion_delta = clip_pose.GetRootMotionDelta();
			Transform& trg_root_motion_delta = data->final_pose.pose_data.GetRootMotionDelta();
			trg_root_motion_delta = src_root_motion_delta;

		}

		data->elasped_time += deltaTime;
		data->definition.update_id = Application::get_instance()->GetUpdateID();

	}

	void* RetargetPoseNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &data->final_pose;
			break;
		}
		}

		return nullptr;
	}

	void RetargetPoseNode::Init(Graph* graph)
	{
		NodeInput input = {};
		input.type = ValueType::Pose;
		input.value_index = 0;
		input.node_id = 0;

		m_inputs.push_back(input);

		NodeOutput output = {};
		output.type = ValueType::Pose;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	PoseNodeResult* RetargetPoseNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}

	// -----------------------------------------------------------------------------------

	// Warp Animation Node -----------------------------------------------------------

	bool WarpAnimationNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		result->final_pose.pose_data.Init(&instance->GetSkeletonInstance());
		result->update_warp = true;

		

		return true;
	}

	void WarpAnimationNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}


		if (data->elasped_time == 0.0f)
		{
			data->start_time = Application::get_instance()->GetClock().GetElapsedTime();
		}

		if (!m_animation)
		{
			return;
		}

		if (data->update_warp)
		{
			get_root_motion_delta(instance);
			//Temp
			UpdateWarp(instance);

			data->update_warp = false;
		}

		Ref<AnimationClip> clip = m_animation;
		Ref<Animation::Skeleton> skeleton = instance->GetGraph()->GetSkeleton();


		float next_frame_time = data->elasped_time + deltaTime;
		if (clip->HasRootMotion())
		{
			AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(clip, skeleton, data->elasped_time, next_frame_time, &data->final_pose.pose_data, true);
			Transform actual_root_motion = AnimationEngine::get_instance()->GetRootMotionDelta(clip, skeleton, data->warped_root_motion, data->elasped_time, next_frame_time, false);
			//data->final_pose.pose_data.GetRootMotionDelta() = actual_root_motion;
			
		}

		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

		auto& channel = clip->GetTracks()[bone->GetStringID()];
		auto& position_track = channel[AnimationDataType::POSITION];
		auto& rotation_track = channel[AnimationDataType::ROTATION];
		
	
		for (int32_t i = 0; i < position_track.size(); i++)
		{
			if (i % 3 != 0)
			{
				continue;
			}
			
			glm::vec3 pos = *(glm::vec3*)(position_track[i].data);
			glm::quat& rot = *(glm::quat*)(rotation_track[i].data);
			
			pos *= 0.1f;

			Transform pose(pos, rot);

			//Debugger::get_instance()->DrawDebugSphere(0.4f, 2, pose.GetLocalMatrix(), TRC_COL32_WHITE);
		}

		int32_t index = -1;
		for (Transform& warp_pose : data->warped_root_motion)
		{
			++index;
			if (index % 3 != 0)
			{
				continue;
			}

			glm::vec3 pos = warp_pose.GetPosition();
			glm::quat rot = warp_pose.GetRotation();

			pos *= 0.1f;

			Transform pose(pos, rot);

			//Debugger::get_instance()->DrawDebugSphere(0.4f, 2, pose.GetLocalMatrix(), TRC_COL32(0, 255 , 0, 255));
		}

		

		data->elasped_time = next_frame_time;
		data->definition.update_id = Application::get_instance()->GetUpdateID();

	}

	bool WarpAnimationNode::get_root_motion_delta(GraphInstance* instance)
	{
		if (!m_animation)
		{
			return false;
		}


		Ref<AnimationClip> clip = m_animation;

		if (!clip->HasRootMotion())
		{
			return false;
		}

		Ref<Animation::Skeleton> skeleton = instance->GetGraph()->GetSkeleton();
		RootMotionInfo& root_motion_info = clip->GetRootMotionInfo();
		Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

		auto& channel = clip->GetTracks()[bone->GetStringID()];
		auto& position_track = channel[AnimationDataType::POSITION];
		auto& rotation_track = channel[AnimationDataType::ROTATION];


		std::vector<Transform>& delta_transforms = m_deltaTransforms;

		delta_transforms.clear();
		delta_transforms.resize(position_track.size());

		delta_transforms[0] = Transform::Identity();
		for (int32_t i = 1; i < position_track.size(); i++)
		{
			glm::vec3 prev_pos = *(glm::vec3*)(position_track[i - 1].data);
			glm::quat& prev_rot = *(glm::quat*)(rotation_track[i - 1].data);

			glm::vec3 pos = *(glm::vec3*)(position_track[i].data);
			glm::quat& rot = *(glm::quat*)(rotation_track[i].data);

			glm::vec3 delta_pos = pos - prev_pos;
			glm::quat delta_rot = rot * glm::inverse(prev_rot);

			delta_transforms[i] = Transform(delta_pos, delta_rot);

		}

		return true;
	}

	bool WarpAnimationNode::UpdateWarp(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		Scene* scene = instance->GetSkeletonInstance().GetScene();


		WarpSection section_00 = {};
		section_00.start_frame = 16;
		section_00.end_frame = 24;
		//section_00.target = world_pos;

		WarpSection section_0 = {};
		section_0.start_frame = 25;
		section_0.end_frame = 40;
		//section_0.target = world_pos_0;

		std::vector<WarpSection> sections;
		//sections.push_back(section_00);
		//sections.push_back(section_0);


		Ref<Animation::Skeleton> skeleton = instance->GetGraph()->GetSkeleton();
		std::vector<Transform> actual_root_motion;
		m_animation->GetRootMotionData(actual_root_motion, skeleton);

		std::vector<Transform>& warped_motion = data->warped_root_motion;

		warped_motion = actual_root_motion;

		Transform entity_global_pose = scene->GetEntityGlobalPose(scene->GetEntity(instance->GetEntityHandle()));

		for (WarpSection& section : sections)
		{

			glm::vec3 local_target = entity_global_pose.Inverse().GetLocalMatrix() * glm::vec4(section.target, 1.0f);
			glm::vec3 offset = local_target - warped_motion[section.end_frame].GetPosition();
			glm::vec3 scale = local_target / warped_motion[section.end_frame].GetPosition();

			int32_t num_animation_frames = actual_root_motion.size();
			int32_t num_warp_frames = section.end_frame - section.start_frame;

			for (int32_t i = 0; i <= num_warp_frames; i++)
			{
				int32_t frame_index = i + section.start_frame;
				float t = float(i) / float(num_warp_frames);
				t = glm::smoothstep(0.0f, 1.0f, t);

				glm::vec3 frame_offset = offset * t;
				warped_motion[frame_index].Translate(frame_offset);

			}

			if (section.end_frame < (num_animation_frames - 1))
			{

				for (int32_t i = (section.end_frame + 1); i < num_animation_frames; i++)
				{
					glm::vec3 new_pos = warped_motion[i - 1].GetPosition() + m_deltaTransforms[i].GetPosition();

					warped_motion[i].SetPosition(new_pos);
				}
			}
		}

		


		return true;
	}

	void* WarpAnimationNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &data->final_pose;
			break;
		}
		}

		return nullptr;
	}

	void WarpAnimationNode::Init(Graph* graph)
	{
		NodeOutput output = {};
		output.type = ValueType::Pose;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	PoseNodeResult* WarpAnimationNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}

	

	// ---------------------------------------------------------------------------------------------

	
	// Motion Matching node -------------------------------------------

	bool MotionMatchingNode::Instanciate(GraphInstance* instance)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* result = new RuntimeData;
		instance_data_set[this] = result;

		result->final_pose.pose_data.Init(m_skeleton);
		result->src_prev_pose.Init(m_skeleton);
		result->trg_pose.Init(m_skeleton);
		result->trg_prev_pose.Init(m_skeleton);

		result->time_accummulator = 1.5f;//NOTE: So as to ensure that there will a pose search at the first frame
		result->inertializer.Initialize(m_skeleton.get());

		return true;
	}

	void MotionMatchingNode::Update(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		if (data->definition.update_id == Application::get_instance()->GetUpdateID())
		{
			return;
		}


		if (data->elasped_time == 0.0f)
		{
			data->start_time = Application::get_instance()->GetClock().GetElapsedTime();
		}

		if (!m_skeleton)
		{
			return;
		}
		
		FindNewPose(instance, deltaTime);

		Ref<Animation::Skeleton> skeleton = m_skeleton;
		
		if (data->current_animation)
		{
			RootMotionInfo& root_motion_info = data->current_animation->GetRootMotionInfo();
			Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

			auto& channel = data->current_animation->GetTracks()[bone->GetStringID()];

			float next_frame_time = data->elasped_time + deltaTime;

			AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(data->last_frame_index, data->current_animation, skeleton, data->elasped_time, next_frame_time, &data->trg_pose, true);
			data->current_index = data->current_clip_index + AnimationEngine::get_instance()->GetFrameIndex(data->last_frame_index, data->current_animation, channel, AnimationDataType::POSITION, next_frame_time, true);
			int32_t frame_in_clip = data->current_index - data->current_clip_index;
			data->last_frame_index = frame_in_clip;
			data->elasped_time = next_frame_time;

			data->inertializer.Update(deltaTime, &data->final_pose.pose_data, &data->trg_pose, skeleton.get());
			data->final_pose.pose_data.GetRootMotionDelta() = data->trg_pose.GetRootMotionDelta();
		}


		

		
		
		data->time_accummulator += deltaTime;
		data->definition.update_id = Application::get_instance()->GetUpdateID();

	}

	void* MotionMatchingNode::GetValueInternal(GraphInstance* instance, uint32_t value_index)
	{
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		switch (value_index)
		{
		case 0:
		{
			return &data->final_pose;
			break;
		}
		}

		return nullptr;
	}

	void MotionMatchingNode::Init(Graph* graph)
	{
		NodeOutput output = {};
		output.type = ValueType::Pose;
		output.value_index = 0;

		m_outputs.push_back(output);
	}

	PoseNodeResult* MotionMatchingNode::GetFinalPose(GraphInstance* instance)
	{
		return (PoseNodeResult*)GetValueInternal(instance);
	}

	void MotionMatchingNode::FindNewPose(GraphInstance* instance, float deltaTime)
	{
		std::unordered_map<UUID, Node*>& nodes = instance->GetGraph()->GetNodes();
		std::unordered_map<Node*, void*>& instance_data_set = instance->GetNodesData();

		RuntimeData* data = reinterpret_cast<RuntimeData*>(instance_data_set[this]);

		Scene* scene = instance->GetSkeletonInstance().GetScene();
		Entity obj = scene->GetEntity(instance->GetEntityHandle());

		if (!obj.HasComponent<MotionMatchingComponent>())
		{
			TRC_ERROR("Entity must have a motion matching component, entity name: {}, Function: {}", obj.GetComponent<TagComponent>().GetTag(), __FUNCTION__);
			return;
		}

		MotionMatchingComponent& comp = obj.GetComponent<MotionMatchingComponent>();

		if (data->time_accummulator > comp.update_frequency)
		{
			// Get current pose feature
			MotionMatching::FeatureData* feature = m_matcher.GetDatabase()->GetData(data->current_index);

			// Generate query vector
			MotionMatching::FeatureData query_vector = *feature;
			query_vector.future_root_positions = comp.trajectory_positions;
			query_vector.future_root_orientation = comp.trajectory_orientations;

			// Search for pose;
			int32_t pose_index = m_matcher.SearchPose(query_vector, comp.trajectory_weight, comp.pose_weight, comp.normalized_search);
			MotionMatching::FeatureData* pose_feature = m_matcher.GetDatabase()->GetData(pose_index);

			const int32_t ignore_surrounding_frames = 15;
			if (pose_index != data->current_index && abs(pose_index - data->current_index) > ignore_surrounding_frames)
			{
				AnimationClip* new_clip = m_matcher.GetDatabase()->GetAnimation(pose_feature->clip_index).get();
				if (data->current_animation)
				{
					float dt = 1.0f / float(data->current_animation->GetSampleRate());
					{
						int32_t frame_in_clip = data->current_index - data->current_clip_index;
						int32_t prev_frame = frame_in_clip - 1;
						if (frame_in_clip == 0)
						{
							prev_frame = 0;//TODO: Allow the prev frame index to wrap around the number of frames avaliable in the clip
						}

						Ref<Animation::Skeleton> skeleton = m_skeleton;
						RootMotionInfo& root_motion_info = data->current_animation->GetRootMotionInfo();
						Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

						auto& channel = data->current_animation->GetTracks()[bone->GetStringID()];
						auto& position_track = channel[AnimationDataType::POSITION];
						AnimationFrameData& frame_data = position_track[frame_in_clip];
						AnimationFrameData& prev_frame_data = position_track[prev_frame];

						AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(data->last_frame_index, data->current_animation, skeleton, prev_frame_data.time_point - dt, prev_frame_data.time_point, &data->src_prev_pose, true);
						AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(data->last_frame_index, data->current_animation, skeleton, frame_data.time_point, frame_data.time_point + dt, &data->final_pose.pose_data, true);
					};

					{
						int32_t frame_in_clip = pose_index - pose_feature->clip_index;

						int32_t prev_frame = frame_in_clip - 1;
						if (frame_in_clip == 0)
						{
							prev_frame = 0;//TODO: Allow the prev frame index to wrap around the number of frames avaliable in the clip
						}

						Ref<Animation::Skeleton> skeleton = m_skeleton;
						RootMotionInfo& root_motion_info = new_clip->GetRootMotionInfo();
						Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

						auto& channel = new_clip->GetTracks()[bone->GetStringID()];
						auto& position_track = channel[AnimationDataType::POSITION];

						AnimationFrameData& frame_data = position_track[frame_in_clip];
						AnimationFrameData& prev_frame_data = position_track[prev_frame];

						AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(prev_frame, new_clip, skeleton, prev_frame_data.time_point - dt, prev_frame_data.time_point, &data->trg_prev_pose, true);

						AnimationEngine::get_instance()->SampleClipWithRootMotionDelta(frame_in_clip, new_clip, skeleton, frame_data.time_point, frame_data.time_point + dt, &data->trg_pose, true);
						

						data->inertializer.Init(&data->src_prev_pose, &data->final_pose.pose_data, &data->trg_prev_pose, &data->trg_pose, skeleton.get(), dt);
					};

				}


				{
					// Set current pose data
					data->current_index = pose_index;
					data->current_clip_index = pose_feature->clip_index;
					data->current_animation = new_clip;

					int32_t frame_in_clip = data->current_index - data->current_clip_index;
					Ref<Animation::Skeleton> skeleton = m_skeleton;
					RootMotionInfo& root_motion_info = data->current_animation->GetRootMotionInfo();
					Animation::Bone* bone = skeleton->GetBone(root_motion_info.root_bone_index);

					auto& channel = data->current_animation->GetTracks()[bone->GetStringID()];
					auto& position_track = channel[AnimationDataType::POSITION];

					AnimationFrameData& frame_data = position_track[frame_in_clip];
					data->last_frame_index = frame_in_clip;
					data->elasped_time = frame_data.time_point;
				};
			}
			else
			{

			}


			data->time_accummulator = 0.0f;
		}
	}

	// ------------------------------------------


	// --------------------------------------------------------------


}