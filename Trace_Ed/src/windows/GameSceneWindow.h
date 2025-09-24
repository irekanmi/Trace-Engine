#pragma once

#include "EditorWindow.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

#include "glm/glm.hpp"

namespace trace {
	
	class HierachyPanel;
	class InspectorPanel;

	enum EditorState
	{
		SceneEdit, ScenePlay, SceneStimulate
	};

	class GameSceneWindow : public EditorWindow
	{

	public:

		bool OnCreate(TraceEditor* editor, const std::string& window_name);
		virtual void OnDestroy(TraceEditor* editor) override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnRender(float deltaTime) override;
		virtual void OnWindowOpen() override;
		virtual void OnWindowLeave() override;
		virtual void OnWindowClose() override;
		virtual void DockChildWindows() override;

		virtual void OnEvent(Event* p_event) override;
		void HandleKeyPressed(KeyPressed* p_event);
		void HandleKeyRelesed(KeyReleased* p_event);

		virtual void RenderViewport(std::vector<void*>& texture_handles) override;

		void RenderSceneToolBar();
		void CloseCurrentScene();
		bool CreateScene(const std::string& file_path);
		void LoadScene(const std::string& file_path);
		void NewScene();
		void SaveScene();
		std::string SaveSceneAs();
		void OpenScene(std::string& path);
		void OnScenePlay();
		void OnSceneStimulate();
		void OnSceneStop();
		void OnGameStart();
		void OnGameStop();
		Ref<Scene> GetCurrentScene() { return m_currentScene; }
		Ref<Scene> GetEditScene() { return m_editScene; }
		Camera& GetEditorCamera() { return m_editorCamera; }
		EditorState GetEditorState() { return m_currentState; }
		glm::vec2 GetViewportSize() { return m_viewportSize; }
		bool SetNextScene(Ref<Scene> scene);
		void HandleEntityDebugDraw();

	private:
		void start_current_scene();
		void stop_current_scene();

	private:
		HierachyPanel* m_hierachyPanel;
		InspectorPanel* m_inspectorPanel;

		glm::vec2 m_viewportSize;
		bool m_viewportFocused;
		bool m_viewportHovered;

		Camera m_editorCamera;
		Ref<Scene> m_currentScene;
		Ref<Scene> m_editScene;
		Ref<Scene> m_editSceneDuplicate;
		Ref<Scene> m_nextScene;
		int gizmo_mode = -1;
		EditorState m_currentState = EditorState::SceneEdit;


		std::string m_currentScenePath;
		std::string m_sceneToOpen;
		std::string m_currentSceneName;

		int32_t scene_render_graph_index = -1;

		bool m_fullScreen = false;
		bool m_stopCurrentScene = false;

	protected:

	};

}
