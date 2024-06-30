#include "pch.h"

#include "AnimationState.h"
#include "core/Application.h"
#include "Animation.h"

namespace trace {

	//TODO: Find a better location(file) for function definition
	const char* get_animation_data_type_string(AnimationDataType type)
	{

		switch (type)
		{
		case AnimationDataType::NONE:
		{
			return "None";
		}
		case AnimationDataType::POSITION:
		{
			return "Position";
		}
		case AnimationDataType::ROTATION:
		{
			return "Rotation";
		}
		case AnimationDataType::SCALE:
		{
			return "Scale";
		}
		case AnimationDataType::TEXT_INTENSITY:
		{
			return "Text Intensity";
		}
		case AnimationDataType::LIGHT_INTENSITY:
		{
			return "Light Intensity";
		}
		case AnimationDataType::IMAGE_COLOR:
		{
			return "Image Color";
		}
		}

		return "";
	}

	

	AnimationState::AnimationState()
		:m_loop(false)
	{
	}

	AnimationState::~AnimationState()
	{
	}

	void AnimationState::Play()
	{

		

		switch (m_playing)
		{
		case 0:
		{
			m_playing = 1;
			m_startTime = Application::get_instance()->GetClock().GetElapsedTime();
			break;
		}
		case 2: 
		{
			m_playing = 1;
			m_startTime = Application::get_instance()->GetClock().GetElapsedTime() - m_elaspedTime;
			break;
		}
		}
	}

	void AnimationState::Pause()
	{
		m_playing = 2;
	}

	void AnimationState::Stop()
	{
		m_playing = 0;
		m_startTime = 0.0f;
		m_elaspedTime = 0.0f;
	}

	bool AnimationState::IsPlaying()
	{
		return m_playing == 1;
	}

}