#include "pch.h"
#include "AnimationGraph.h"

bool trace::AnimationGraph::HasAnimationClip(Ref<AnimationClip> clip)
{

    for (AnimationState& state : m_states)
    {
        Ref<AnimationClip> current_state_clip = state.GetAnimationClip();
        if ( current_state_clip && current_state_clip->Compare(clip.get()) )
        {
            return true;
        }
    }

    return false;
}

void trace::AnimationGraph::SetAsRuntime()
{
    for (AnimationState& state : m_states)
    {
        state.SetAsRuntime();
    }

}

void trace::AnimationGraph::Start()
{
    m_started = true;
    m_currrentStateIndex = m_startIndex;

    AnimationState& current_state = m_states[m_currrentStateIndex];

    if (current_state.GetAnimationClip())
    {
        current_state.Play();
    }
}
