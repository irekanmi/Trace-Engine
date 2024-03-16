#pragma once


#include "scene/UUID.h"
#include "resource/Resource.h"

#include <vector>
#include <unordered_map>

namespace trace {

	enum class AnimationDataType
	{
		NONE,
		POSITION,
		ROTATION,
		SCALE,
		TEXT_INTENSITY,
		LIGHT_INTENSITY,
	};

	struct AnimationFrameData
	{
		char data[16] = {0};// member that holds the frame data;
		float time_point = 0.0f;
	};

	struct AnimationTrack
	{
		UUID entity_handle;
		AnimationDataType channel_type;
		std::vector<AnimationFrameData> channel_data;
	};


	class AnimationClip : public Resource
	{

	public:
		~AnimationClip(){}

		std::vector<AnimationTrack>& GetTracks() { return m_tracks; }
		float GetDuration() { return m_duration; }

		void SetDuration(float duration) { m_duration = duration; }

	private:
		float m_duration = 0.0f;
		float m_sampleRate = 1 / 30.0f;
		std::vector<AnimationTrack> m_tracks;

	protected:


	};

}
