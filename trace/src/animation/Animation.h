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
		MAX
	};

	const char* get_animation_data_type_string(AnimationDataType type);

	struct AnimationFrameData
	{
		char data[16] = {0};// member that holds the frame data;
		float time_point = 0.0f;
	};

	struct AnimationTrack
	{
		AnimationDataType channel_type;
		std::vector<AnimationFrameData> channel_data;
	};


	class AnimationClip : public Resource
	{

	public:
		~AnimationClip(){}

		std::unordered_map<UUID, std::vector<AnimationTrack>>& GetTracks() { return m_tracks; }
		float GetDuration() { return m_duration; }
		int GetSampleRate() { return m_sampleRate; }

		void SetDuration(float duration) { m_duration = duration; }
		void SetSampleRate(int rate) { m_sampleRate = rate; }
		void SetTracks(std::unordered_map<UUID, std::vector<AnimationTrack>>& new_tracks) { m_tracks = std::move(new_tracks); }

	private:
		float m_duration = 1.0f;
		int m_sampleRate = 30;
		std::unordered_map<UUID,std::vector<AnimationTrack>> m_tracks;

	protected:


	};

}
