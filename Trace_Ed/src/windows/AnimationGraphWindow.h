#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "animation/AnimationGraph.h"

namespace trace {

	class HierachyPanel;
	class AnimationGraphEditor;

	class AnimationGraphWindow : public EditorWindow
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
		Ref<Animation::Graph> m_graph;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		HierachyPanel* m_hierachy;
		AnimationGraphEditor* m_editor;
		glm::vec2 m_viewportSize;
		std::string hierachy_name;
		std::string graph_editor_name;
		std::string viewport_name;
		std::string tool_bar_name;
		int gizmo_mode = 1;
		std::string graph_path;
		bool has_prefab = false;
		bool is_playing = false;
		AnimationGraphController* graph_controller = nullptr;
		UUID entity_handle = 0;


	protected:

	};

}
