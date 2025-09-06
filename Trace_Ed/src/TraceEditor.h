
#pragma once

#include "EditorRenderComposer.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "windows/EditorWindow.h"

#include "project/Project.h"
#include <filesystem>
#include "windows/GameSceneWindow.h"
#include "imgui.h"

namespace trace {
	class HierachyPanel;
	class InspectorPanel;
	class ContentBrowser;
	class AnimationPanel;
	class AnimationGraphEditor;
	class AnimationSequencer;
	class Importer;

	class GameSceneWindow;


	class TraceEditor
	{

	public:

		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float delaTime);
		void RenderViewport(std::vector<void*>& texture_handles);

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
		void HandleKeyPressed(KeyPressed* p_event);
		void HandleKeyRelesed(KeyReleased* p_event);
		ContentBrowser* GetContentBrowser() { return m_contentBrowser; }
		Importer* GetImporter() { return m_importer; }
		AllProjectAssets& GetAllProjectAssets() { return m_allAssets; }
		Ref<Project> GetCurrentProject() { return m_currentProject; }
		EditorState GetEditorState() { return scene_window->GetEditorState(); }
		bool SetNextScene(Ref<Scene> scene);
		void AddWindowToDockspace(const std::string& window_name);

		template<typename T, typename ...Args>
		T* CreateEditorWindow(const std::string& name, Args ...args)
		{
			T* result = new T;//TODO: Use custom allocator
			if (result->OnCreate(this, name, std::forward<Args>(args)...))
			{
				m_windows.push_back(result);
				AddWindowToDockspace(name);
				return result;
			}
			else
			{
				delete result;//TODO: Use custom allocator
			}
			return nullptr;
		}

		//TEMP ========================
		void Update_Tester(float deltaTime);
		void OpenSkeleton(std::string& path);
		void OpenFeatureDB(std::string& path);
		void OpenMMTInfo(std::string& path);
		void OpenPrefab(std::string& path);
		void OpenScene(std::string& path);
		void OpenAnimationGraph(std::string& path);
		void OpenAnimationSequence(std::string& path);
		void OpenAnimationClip(std::string& path);


		
	private:
		AllProjectAssets m_allAssets;	


		EditorRenderComposer* m_renderComposer = nullptr;
		ContentBrowser* m_contentBrowser;
		Importer* m_importer;

		Ref<Project> m_currentProject;

	private:


	private:
		bool CreateProject(const std::string& dir, const std::string& name);
		bool OpenProject();
		bool LoadProject(const std::string& file_path);
		bool CloseProject();
		void ReloadProjectAssembly();
		void BuildProject();
		void RenderUtilsWindows(float deltaTime);

		void ProjectSettings();

		bool p_createProject = false;
		bool p_projectSettings = false;

		GameSceneWindow* scene_window = nullptr;
		std::vector<EditorWindow*> m_windows;
		float m_deltaTime = 0.0f;
		ImGuiID main_dockspace;
		ImGuiID main_dockspace_Top;
		ImGuiID main_dockspace_Bottom;

	protected:
	};

}
