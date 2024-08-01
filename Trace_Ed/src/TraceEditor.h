#pragma once

#include "EditorRenderComposer.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

#include "project/Project.h"
#include <filesystem>

namespace trace {
	class HierachyPanel;
	class InspectorPanel;    
	class ContentBrowser;
	class AnimationPanel;
	class AnimationGraphEditor;
	class Importer;

	enum EditorState
	{
		SceneEdit, ScenePlay, SceneStimulate
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

		std::string DrawModelsPopup();
		std::string DrawMaterialsPopup();
		bool DrawTexturesPopup(std::string& result);
		bool DrawPipelinesPopup(std::string& result);
		bool DrawShadersPopup(std::string& result);
		bool InputTextPopup(const std::string& label, std::string& result);

		static TraceEditor* get_instance();

		struct AllProjectAssets
		{
			std::unordered_set<std::filesystem::path> models;
			std::unordered_set<std::filesystem::path> textures;
			std::unordered_set<std::filesystem::path> meshes;
			std::unordered_set<std::filesystem::path> materials;
			std::unordered_set<std::filesystem::path> pipelines;
			std::unordered_set<std::filesystem::path> shaders;
			std::unordered_set<std::filesystem::path> scenes;
		};

	public:
		void DrawGizmo();
		void DrawGrid(CommandList& cmd_list);
		void CloseCurrentScene();
		bool CreateScene(const std::string& file_path);
		void LoadScene(const std::string& file_path);
		void NewScene();
		void SaveScene();
		std::string SaveSceneAs();
		std::string OpenScene();
		void OpenScene(std::string& path);
		void HandleKeyPressed(KeyPressed* p_event);
		void HandleKeyRelesed(KeyReleased* p_event);
		void OnScenePlay();
		void OnSceneStimulate();
		void OnSceneStop();

		HierachyPanel* GetHierachyPanel() { return m_hierachyPanel; }
		InspectorPanel* GetInspectorPanel() { return m_inspectorPanel; }
		ContentBrowser* GetContentBrowser() { return m_contentBrowser; }
		AnimationPanel* GetAnimationPanel() { return m_animPanel; }
		AnimationGraphEditor* GetAnimationGraphEditor() { return m_animGraphEditor; }
		Importer* GetImporter() { return m_importer; }
		AllProjectAssets& GetAllProjectAssets() { return m_allAssets; }
		Ref<Project> GetCurrentProject() { return m_currentProject; }
		Ref<Scene> GetCurrentScene() { return m_currentScene; }
		Ref<Scene> GetEditScene() { return m_editScene; }
		Camera& GetEditorCamera() { return m_editorCamera; }
		EditorState GetEditorState() { return m_currentState; }
		glm::vec2 GetViewportSize() { return m_viewportSize; }


		
	private:
		AllProjectAssets m_allAssets;	


		EditorRenderComposer* m_renderComposer = nullptr;
		HierachyPanel* m_hierachyPanel;
		InspectorPanel* m_inspectorPanel;
		ContentBrowser* m_contentBrowser;
		AnimationPanel* m_animPanel;
		AnimationGraphEditor* m_animGraphEditor;
		Importer* m_importer;

		glm::vec2 m_viewportSize;
		bool m_viewportFocused;
		bool m_viewportHovered;

		Camera m_editorCamera;
		Ref<Scene> m_currentScene;
		Ref<Scene> m_editScene;
		Ref<Scene> m_editSceneDuplicate;
		Ref<Project> m_currentProject;
		int gizmo_mode = -1;
		EditorState m_currentState = EditorState::SceneEdit;
		

		std::string m_currentScenePath;

	private:
		bool CreateProject(const std::string& dir, const std::string& name);
		bool OpenProject();
		bool LoadProject(const std::string& file_path);
		bool CloseProject();
		void ReloadProjectAssembly();
		void BuildProject();

		void ProjectSettings();

		bool p_createProject = false;
		bool p_projectSettings = false;

		bool m_fullScreen = false;

	protected:
	};

}
