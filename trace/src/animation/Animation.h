#pragma once


#include "scene/UUID.h"
#include "resource/Resource.h"
#include "animation/Skeleton.h"
#include "resource/Ref.h"
#include "render/Transform.h"
#include "core/Coretypes.h"

#include <vector>
#include <unordered_map>
#include <string>

namespace trace {

	enum class AnimationDataType
	{
		NONE,
		POSITION,
		ROTATION,
		SCALE,
		TEXT_INTENSITY,
		LIGHT_INTENSITY,
		IMAGE_COLOR,
		MAX
	};

	enum class AnimationClipType
	{
		SEQUENCE,
		SKELETAL_ANIMATIOM,
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

	using AnimationDataTrack = std::unordered_map< AnimationDataType, std::vector<AnimationFrameData>>;


	class AnimationClip : public Resource
	{

	public:
		~AnimationClip(){}

		std::unordered_map<StringID, AnimationDataTrack>& GetTracks() { return m_tracks; }
		float GetDuration() { return m_duration; }
		int GetSampleRate() { return m_sampleRate; }
		AnimationClipType GetType() { return m_type; }

		void SetDuration(float duration) { m_duration = duration; }
		void SetSampleRate(int rate) { m_sampleRate = rate; }
		void SetTracks(std::unordered_map<StringID, AnimationDataTrack>& new_tracks) { m_tracks = std::move(new_tracks); }
		void SetType(AnimationClipType type) { m_type = type; }


		bool Compare(AnimationClip* other);

	private:
		float m_duration = 1.0f;
		int m_sampleRate = 30;
		std::unordered_map<StringID, AnimationDataTrack> m_tracks;
		AnimationClipType m_type = AnimationClipType::SKELETAL_ANIMATIOM;

	protected:


	};

	

}
