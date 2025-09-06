#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "animation/AnimationSequence.h"

namespace trace {

	class HierachyPanel;
	class AnimationSequencer;

	class SequenceWindow : public EditorWindow
	{

	public:

		bool OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path);
		virtual void OnDestroy(TraceEditor* editor);
		virtual void OnUpdate(float deltaTime);
		virtual void OnRender(float deltaTime);
		virtual void DockChildWindows();
		virtual void RenderViewport(std::vector<void*>& texture_handles);
		virtual void OnEvent(Event* p_event);

	private:
		Ref<Animation::Sequence> m_sequence;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		HierachyPanel* m_hierachy;
		AnimationSequencer* m_editor;
		glm::vec2 m_viewportSize;
		std::string hierachy_name;
		std::string sequence_editor_name;
		std::string viewport_name;
		int gizmo_mode = 1;
		std::string sequence_path;
		bool has_prefab = false;


	protected:

	};

}
