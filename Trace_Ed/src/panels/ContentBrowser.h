#pragma once

#include "resource/Ref.h"
#include "render/GTexture.h"
#include "render/Material.h"
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
		std::filesystem::path m_editMaterialPath;
		MaterialData m_materialDataCache;
		

		TraceEditor* m_editor;
	protected:
		friend class TraceEditor;

	};

}
