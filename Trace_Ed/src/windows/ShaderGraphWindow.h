#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "shader_graph/ShaderGraph.h"

namespace trace {

	class GenericGraphEditor;
	class InspectorPanel;

	class ShaderGraphWindow : public EditorWindow
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
		Ref<ShaderGraph> m_shaderGraph;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		GenericGraphEditor* m_editor;
		InspectorPanel* m_inspector;
		glm::vec2 m_viewportSize;
		std::string graph_editor_name;
		std::string inspector_name;
		std::string viewport_name;
		int gizmo_mode = 1;
		std::string graph_path;
		Ref<MaterialInstance> m_material;
		ShaderGraphInstance graph_instance;
		bool can_render = true;
		UUID visual_id = 0;



	protected:

	};

}
