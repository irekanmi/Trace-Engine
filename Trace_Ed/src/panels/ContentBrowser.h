#pragma once

#include "resource/Ref.h"
#include "render/GTexture.h"
#include "render/Material.h"
#include "render/Font.h"
#include "scene/UUID.h"

#include <filesystem>
#include <vector>

namespace trace {




	class TraceEditor;

	class ContentBrowser
	{

	public:

		bool Init();
		void Shutdown();
		void Render(float deltaTime);
		void OnDirectoryChanged();
		void ProcessDirectory();
		void ProcessAllDirectory();
		void OnWindowPopup();
		void OnItemPopup(std::filesystem::path& path);
		void DrawEditMaterial();
		void DrawEditFont();
		void DrawEditPipeline();

	public:
		//TODO: Check if there is a better way to store uuid and paths
		std::unordered_map<std::string, UUID> all_files_id;
		std::unordered_map<UUID, std::filesystem::path> all_id_path;

	private:
		std::filesystem::path m_currentDir;
		std::vector<std::filesystem::path> m_dirContents;
		Ref<GTexture> directory_icon;
		Ref<GTexture> default_icon;

		// Material edit
		Ref<MaterialInstance> m_editMaterial;
		Ref<GPipeline> m_editMaterialPipe;
		std::filesystem::path m_editMaterialPath;
		MaterialData m_materialDataCache;
		bool m_editMaterialPipeChanged = false;

		
		// Font edit
		Ref<Font> m_editFont;

		//Pipeline edit
		Ref<GPipeline> m_editPipeline;
		PipelineStateDesc m_editPipeDesc;
		std::filesystem::path m_editPipePath;
		PipelineType m_editPipeType;

		TraceEditor* m_editor;
	protected:
		friend class TraceEditor;

	};

}
