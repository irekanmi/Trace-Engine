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

		struct FrameIndex
		{
			int index = -1;
			int current_fd_index = -1;
		};

		AnimationPanel();
		~AnimationPanel() {}

		bool Init();
		void Shutdown();

		void Render(float deltaTime);
		bool Recording();
		void SetFrameData(UUID id, AnimationDataType type, void* data, int size);

		Ref<AnimationClip> GetAnimationClip() { return m_currentClip; }
		int32_t GetCurrentFrame() { return m_currentFrame; }
		int32_t GetStartFrame() { return m_startFrame; }
		int32_t GetEndFrame() { return m_endFrame; }
		int32_t GetLastSelectedFrame() { return m_lastSelectedFrame; }

		void SetAnimationClip(Ref<AnimationClip> clip);

	private:
		void generate_tracks();
		void play();
		void pause();
		void animation_data_ui(AnimationDataType type, void* data);

		float m_elapsed = 0.0f;
		int32_t m_currentFrame = 0;
		int32_t m_startFrame = -10;
		int32_t m_endFrame = 64;
		int32_t m_lastSelectedFrame = -1;

		bool m_doDelete = false;

		Ref<AnimationClip> m_currentClip;
		State m_state;
		std::unordered_map<UUID, std::unordered_map<AnimationDataType,std::vector<FrameIndex>>> m_currentTracks;

	protected:
	};

}