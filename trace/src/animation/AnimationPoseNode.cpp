#include "pch.h"

#include "animation/AnimationPoseNode.h"
#include "core/Application.h"
#include "animation/AnimationEngine.h"
#include "scene/Entity.h"
#include "animation/AnimationBlend.h"
#include "animation/AnimationGraph.h"


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


	// Animation Sample Node -----------------------------------------------------------

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

				glm::mat4 offset = target_bind_pose * inv_source_bind_pose;

				glm::mat4 animation_pose = clip_pose.GetBoneGlobalPose(m_skeleton, source_bone_index);

				glm::mat4 pose_result = offset * animation_pose;

				if (target_bone.GetParentIndex() != -1)
				{
					glm::mat4 target_parent_pose = data->final_pose.pose_data.GetBoneGlobalPose(target_bone.GetParentIndex());
					pose_result = glm::inverse(target_parent_pose) * pose_result;
				}

				std::vector<Transform>& target_local_pose = data->final_pose.pose_data.GetLocalPose();
				target_local_pose[target_bone_index] = Transform(pose_result);

				++index;
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



	// ---------------------------------------------------------------------------------------------

}