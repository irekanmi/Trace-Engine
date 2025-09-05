#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "resource/Prefab.h"

namespace trace {

	class HierachyPanel;
	class InspectorPanel;

	class PrefabWindow : public EditorWindow
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
		Camera m_camera;
		Ref<Prefab> m_prefab;
		int32_t view_index;
		HierachyPanel* m_hierachy;
		InspectorPanel* m_inspector;
		glm::vec2 m_viewportSize;
		std::string hierachy_name;
		std::string inspector_name;
		std::string viewport_name;
		int gizmo_mode = 1;
		std::string prefab_path;


	protected:

	};

}
