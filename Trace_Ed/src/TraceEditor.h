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
		
		//std::filesystem::path current_project_path = "../assets"; //Temp
		std::filesystem::path current_project_path = "C:\\Dev\\VisualSutdio\\Cpp\\Trace\\assets"; //Temp
		std::string current_scene_path;
		static TraceEditor* s_instance;
	protected:
		friend EditorRenderComposer;
		friend HierachyPanel;
		friend InspectorPanel;
		friend ContentBrowser;
	};

}
