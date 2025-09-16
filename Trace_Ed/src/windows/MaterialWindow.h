#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "render/Material.h"

namespace trace {

	class InspectorPanel;

	class MaterialWindow : public EditorWindow
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
		Ref<MaterialInstance> m_material;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		InspectorPanel* m_inspector;
		glm::vec2 m_viewportSize;
		std::string inspector_name;
		std::string viewport_name;
		int gizmo_mode = 1;
		std::string material_path;
		UUID visual_id = 0;



	protected:

	};

}
