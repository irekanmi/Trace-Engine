#pragma once
#include "EditorRenderComposer.h"
#include "panels/HierachyPanel.h"
#include "panels/InspectorPanel.h"

#include "scene/Scene.h"
#include <filesystem>

namespace trace {

	class TraceEditor
	{

	public:

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float delaTime);
		void RenderViewport(void* texture);

		RenderComposer* GetRenderComposer();

		void OnEvent(Event* p_event);

		static TraceEditor* get_instance();

	private:

	private:
		EditorRenderComposer* m_renderComposer = nullptr;
		HierachyPanel m_hierachyPanel;
		InspectorPanel m_inspectorPanel;

		glm::vec2 m_viewportSize;
		bool m_viewportFocused;
		bool m_viewportHovered;

		Camera editor_cam;
		Ref<Scene> m_currentScene;
		
		std::filesystem::path current_project_path;
		static TraceEditor* s_instance;
	protected:
		friend EditorRenderComposer;
		friend HierachyPanel;
		friend InspectorPanel;
	};

}
