
#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "animation/AnimationNode.h"
#include "animation/AnimationPoseNode.h"
#include "animation/AnimationSequence.h"
#include "animation/AnimationSequenceTrack.h"
#include "animation/SequenceTrackChannel.h"
#include "animation/Bone.h"
#include "animation/Skeleton.h"

#include "reflection/TypeRegistry.h"

#include "scene/Entity.h"
#include "scene/Scene.h"

namespace trace::Animation {

	REGISTER_TYPE(StringID);//TODO: Move this type to a registry file that relates with Coretypes
	REGISTER_TYPE(size_t);//TODO: Move this type to a registry file that relates with Coretypes

	REGISTER_TYPE(ParameterType);

	BEGIN_REGISTER_CLASS(ParameterData)
		REGISTER_TYPE(ParameterData);
		REGISTER_MEMBER(ParameterData, data);
	END_REGISTER_CLASS;


	REGISTER_TYPE(ConditionType);

	BEGIN_REGISTER_CLASS(ConditionData)
		REGISTER_TYPE(ConditionData);
		REGISTER_MEMBER(ConditionData, data);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Condition)
		REGISTER_TYPE(Condition);
		REGISTER_MEMBER(Condition, type);
		REGISTER_MEMBER(Condition, data);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(TransitionSet)
		REGISTER_TYPE(TransitionSet);
		REGISTER_MEMBER(TransitionSet, first);
		REGISTER_MEMBER(TransitionSet, second);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Parameter)
		REGISTER_TYPE(Parameter);
		REGISTER_MEMBER(Parameter, first);
		REGISTER_MEMBER(Parameter, second);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Graph)
		REGISTER_TYPE(Graph);
		REGISTER_MEMBER(Graph, m_nodes);
		REGISTER_MEMBER(Graph, m_rootNode);
		REGISTER_MEMBER(Graph, m_skeleton);
		REGISTER_MEMBER(Graph, m_animationDataSet);
		REGISTER_MEMBER(Graph, m_parameters);
		REGISTER_MEMBER(Graph, m_parameterTranstions);
	END_REGISTER_CLASS;

	REGISTER_TYPE(ValueType);

	BEGIN_REGISTER_CLASS(NodeInput)
		REGISTER_TYPE(NodeInput);
		REGISTER_MEMBER(NodeInput, type);
		REGISTER_MEMBER(NodeInput, node_id);
		REGISTER_MEMBER(NodeInput, value_index);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(NodeOutput)
		REGISTER_TYPE(NodeOutput);
		REGISTER_MEMBER(NodeOutput, type);
		REGISTER_MEMBER(NodeOutput, value_index);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Node)
		REGISTER_TYPE(Node);
		REGISTER_MEMBER(Node, m_inputs);
		REGISTER_MEMBER(Node, m_outputs);
		REGISTER_MEMBER(Node, m_uuid);
	END_REGISTER_CLASS;

	REGISTER_TYPE_PARENT(EntryNode, Node);

	BEGIN_REGISTER_CLASS(GetParameterNode)
		REGISTER_TYPE_PARENT(GetParameterNode, Node);
		REGISTER_MEMBER(GetParameterNode, m_parameterIndex);
	END_REGISTER_CLASS;

	REGISTER_TYPE_PARENT(PoseNode, Node);

	BEGIN_REGISTER_CLASS(StateMachine)
		REGISTER_TYPE_PARENT(StateMachine, PoseNode);
		REGISTER_MEMBER(StateMachine, m_states);
		REGISTER_MEMBER(StateMachine, m_entryNode);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(StateNode)
		REGISTER_TYPE_PARENT(StateNode, PoseNode);
		REGISTER_MEMBER(StateNode, m_name);
		REGISTER_MEMBER(StateNode, m_transitions);
		REGISTER_MEMBER(StateNode, m_stateMachine);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(TransitionNode)
		REGISTER_TYPE_PARENT(TransitionNode, PoseNode);
		REGISTER_MEMBER(TransitionNode, m_fromState);
		REGISTER_MEMBER(TransitionNode, m_targetState);
		REGISTER_MEMBER(TransitionNode, m_duration);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(AnimationSampleNode)
		REGISTER_TYPE_PARENT(AnimationSampleNode, PoseNode);
		REGISTER_MEMBER(AnimationSampleNode, m_animClipIndex);
		REGISTER_MEMBER(AnimationSampleNode, m_duration);
		REGISTER_MEMBER(AnimationSampleNode, m_looping);
	END_REGISTER_CLASS;

	REGISTER_TYPE_PARENT(FinalOutputNode, PoseNode);

	REGISTER_TYPE(SequenceType);

	BEGIN_REGISTER_CLASS(Sequence)
		REGISTER_TYPE(Sequence);
		REGISTER_MEMBER(Sequence, m_tracks);
		REGISTER_MEMBER(Sequence, m_type);
		REGISTER_MEMBER(Sequence, m_duration);
	END_REGISTER_CLASS;

	REGISTER_TYPE(SequenceTrackType);

	BEGIN_REGISTER_CLASS(SequenceTrack)
		REGISTER_TYPE(SequenceTrack);
		REGISTER_MEMBER(SequenceTrack, m_channels);
		REGISTER_MEMBER(SequenceTrack, m_type);
		REGISTER_MEMBER(SequenceTrack, m_stringID);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(AnimationSequenceTrack)
		REGISTER_TYPE_PARENT(AnimationSequenceTrack, SequenceTrack);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SkeletalAnimationTrack)
		REGISTER_TYPE_PARENT(SkeletalAnimationTrack, SequenceTrack);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ActivationTrack)
		REGISTER_TYPE_PARENT(ActivationTrack, SequenceTrack);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SequenceTrackChannel)
		REGISTER_TYPE(SequenceTrackChannel);
		REGISTER_MEMBER(SequenceTrackChannel, m_startTime);
		REGISTER_MEMBER(SequenceTrackChannel, m_duration);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(AnimationChannel)
		REGISTER_TYPE_PARENT(AnimationChannel, SequenceTrackChannel);
		REGISTER_MEMBER(AnimationChannel, m_clip);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SkeletalAnimationChannel)
		REGISTER_TYPE_PARENT(SkeletalAnimationChannel, SequenceTrackChannel);
		REGISTER_MEMBER(SkeletalAnimationChannel, m_clip);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(ActivationChannel)
		REGISTER_TYPE_PARENT(ActivationChannel, SequenceTrackChannel);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Bone)
		REGISTER_TYPE(Bone);
		REGISTER_MEMBER(Bone, m_bindPose);
		REGISTER_MEMBER(Bone, m_boneOffset);
		REGISTER_MEMBER(Bone, m_id);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(Skeleton)
		REGISTER_TYPE(Skeleton);
		REGISTER_MEMBER(Skeleton, m_name);
		REGISTER_MEMBER(Skeleton, m_bones);
		REGISTER_MEMBER(Skeleton, m_rootNodeID);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(SkeletonInstance)
		REGISTER_TYPE(SkeletonInstance);
		REGISTER_MEMBER(SkeletonInstance, m_skeleton);
	END_REGISTER_CLASS;

	BEGIN_REGISTER_CLASS(GraphInstance)
		REGISTER_TYPE(GraphInstance);
		REGISTER_MEMBER(GraphInstance, m_graph);
		REGISTER_MEMBER(GraphInstance, m_skeletonInstance);
	END_REGISTER_CLASS;

}