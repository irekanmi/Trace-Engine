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
	struct AssetsEdit
	{
		// Material edit
		Ref<MaterialInstance> editMaterial;
		Ref<GPipeline> editMaterialPipe;
		std::filesystem::path editMaterialPath;
		MaterialData materialDataCache;
		bool editMaterialPipeChanged = false;


		// Font edit
		Ref<Font> editFont;

		//Pipeline edit
		Ref<GPipeline> editPipeline;
		PipelineStateDesc editPipeDesc;
		std::filesystem::path editPipePath;
		PipelineType editPipeType;
	};

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
		void SetDirectory(const std::string& dir);
		void SerializeImportedAssets();
		void LoadImportedAssets();

		std::unordered_map<std::string, UUID>& GetAllFilesID() { return m_allFilesID; }
		std::unordered_map<UUID, std::filesystem::path> GetUUIDPath() { return m_allIDPath; }
		std::filesystem::path& GetCurrentDirectory() { return m_currentDir; }
		Ref<GTexture> GetDefaultIcon() { return default_icon; }
		AssetsEdit& GetAssetsEdit() { return m_assetsEdit; }
		std::unordered_set<std::string>& GetImportedAssets() { return m_importedAssets; }
		
	private:

	private:
		//TODO: Check if there is a better way to store uuid and paths
		std::unordered_map<std::string, UUID> m_allFilesID;
		std::unordered_map<UUID, std::filesystem::path> m_allIDPath;

		std::unordered_set<std::string> m_importedAssets;

	private:
		std::filesystem::path m_currentDir;
		std::vector<std::filesystem::path> m_dirContents;
		Ref<GTexture> directory_icon;
		Ref<GTexture> default_icon;
		
		AssetsEdit m_assetsEdit;

	protected:

	};

}
