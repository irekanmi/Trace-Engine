#pragma once


#include "animation/Animation.h"
#include "resource/Ref.h"

#include <vector>
#include <unordered_map>

namespace trace {

	class TraceEditor;

	class AnimationPanel
	{

	public:
		enum State
		{
			PLAY,
			PAUSE,
			RECORD
		};

		AnimationPanel();
		~AnimationPanel() {}

		bool Init();
		void Shutdown();

		void Render(float deltaTime);
		void SetAnimationClip(Ref<AnimationClip> clip);
		bool Recording();
		void SetFrameData(UUID id, AnimationDataType type, void* data, int size);

		TraceEditor* m_editor;
		int currentFrame = 0;
		int startFrame = -10;
		int endFrame = 64;

		bool doDelete = false;
	private:
		void generate_tracks();
		void play();
		void pause();

		float m_elapsed = 0.0f;


		Ref<AnimationClip> m_currentClip;
		State m_state;
		std::unordered_map<UUID, std::unordered_map<AnimationDataType,std::vector<int>>> m_currentTracks;

	protected:
		friend class TraceEditor;
	};

}