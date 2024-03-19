#pragma once
#include "EditorRenderComposer.h"



#include "project/Project.h"
#include <filesystem>

namespace trace {
	class Scene;
	class HierachyPanel;
	class InspectorPanel;    
	class ContentBrowser;
	class AnimationPanel;

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

		struct AllProjectAssets
		{
			std::unordered_set<std::filesystem::path> models;
			std::unordered_set<std::filesystem::path> textures;
			std::unordered_set<std::filesystem::path> meshes;
			std::unordered_set<std::filesystem::path> materials;
			std::unordered_set<std::filesystem::path> pipelines;
			std::unordered_set<std::filesystem::path> shaders;
		};
		AllProjectAssets all_assets;
	public:
		


		EditorRenderComposer* m_renderComposer = nullptr;
		HierachyPanel* m_hierachyPanel;
		InspectorPanel* m_inspectorPanel;
		ContentBrowser* m_contentBrowser;
		AnimationPanel* m_animPanel;

		glm::vec2 m_viewportSize;
		bool m_viewportFocused;
		bool m_viewportHovered;

		Camera editor_cam;
		Ref<Scene> m_currentScene;
		Ref<Scene> m_editScene;
		Ref<Scene> m_editScene_duplicate;
		Ref<Project> current_project;
		int gizmo_mode = -1;
		EditorState current_state = EditorState::SceneEdit;
		

		std::string current_scene_path;
		static TraceEditor* s_instance;

	private:
		bool CreateProject(const std::string& dir, const std::string& name);
		bool OpenProject();
		bool LoadProject(const std::string& file_path);
		bool CloseProject();
		void ReloadProjectAssembly();

		void ProjectSettings();

		bool p_createProject = false;
		bool p_projectSettings = false;


	protected:
		friend EditorRenderComposer;
		friend HierachyPanel;
		friend InspectorPanel;
		friend ContentBrowser;
	};

}
