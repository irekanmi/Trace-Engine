#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "animation/Animation.h"

namespace trace {

	class HierachyPanel;
	class InspectorPanel;
	class AnimationPanel;

	class AnimationWindow : public EditorWindow
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
		Ref<AnimationClip> m_animation;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		HierachyPanel* m_hierachy;
		InspectorPanel* m_inspector;
		AnimationPanel* m_editor;
		glm::vec2 m_viewportSize;
		std::string hierachy_name;
		std::string inspector_name;
		std::string animation_editor_name;
		std::string viewport_name;
		int gizmo_mode = 1;
		std::string animation_path;
		bool has_prefab = false;


	protected:

	};

}
