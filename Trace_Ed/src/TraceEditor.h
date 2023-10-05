#pragma once
#include "EditorRenderComposer.h"
#include "panels/HierachyPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/ContentBrowser.h"

#include "scene/Scene.h"
#include <filesystem>

namespace trace {


	enum EditorState
	{
		SceneEdit, ScenePlay
	};

	class TraceEditor
	{

	public:

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float delaTime);
		void RenderViewport(void* texture);
		void RenderSceneToolBar();

		RenderComposer* GetRenderComposer();

		void OnEvent(Event* p_event);

		static TraceEditor* get_instance();

	private:
		void DrawGizmo();
		void CloseCurrentScene();
		void LoadScene();
		void HandleKeyPressed(KeyPressed* p_event);
		void HandleKeyRelesed(KeyReleased* p_event);
		void OnScenePlay();
		void OnSceneStop();

	private:
		EditorRenderComposer* m_renderComposer = nullptr;
		HierachyPanel m_hierachyPanel;
		InspectorPanel m_inspectorPanel;
		ContentBrowser m_contentBrowser;

		glm::vec2 m_viewportSize;
		bool m_viewportFocused;
		bool m_viewportHovered;

		Camera editor_cam;
		Ref<Scene> m_currentScene;
		Ref<Scene> m_editScene;
		Ref<Scene> m_editScene_duplicate;
		int gizmo_mode = -1;
		EditorState current_state = EditorState::SceneEdit;
		
		std::filesystem::path current_project_path = "../assets"; //Temp
		static TraceEditor* s_instance;
	protected:
		friend EditorRenderComposer;
		friend HierachyPanel;
		friend InspectorPanel;
		friend ContentBrowser;
	};

}
