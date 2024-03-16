#include "pch.h"

#include "AnimationEngine.h"
#include "core/Application.h"


namespace trace {



	bool AnimationEngine::Init()
	{
		return false;
	}

	void AnimationEngine::Shutdown()
	{
	}

	void AnimationEngine::Animate(AnimationState& state)
	{

		if (!state.IsPlaying()) return;

		Ref<AnimationClip> clip = state.GetAnimationClip();

		float _t = Application::get_instance()->GetClock().GetElapsedTime() - state.GetStartTime();

		if (_t > clip->GetDuration() && !state.GetLoop()) return;

		for (AnimationTrack& track : clip->GetTracks())
		{
			AnimationFrameData* _f1 = nullptr;
			AnimationFrameData* _f2 = nullptr;

			//TODO: Find a better way to find which frame to sample
			int i = 0;
			for (AnimationFrameData& data : track.channel_data)
			{
				if (_t >= data.time_point)
				{
					_f1 = &data;
					if (i < track.channel_data.size()) _f2 = &track.channel_data[i];
				}

				++i;
			}
			
		}

	}

}