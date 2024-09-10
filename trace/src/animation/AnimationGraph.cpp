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
