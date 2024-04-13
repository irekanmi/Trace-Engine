#include "pch.h"

#include "ProjectBuilder.h"
#include "serialize/FileStream.h"
#include "serialize/AssetsInfo.h"
#include "serialize/SceneSerializer.h"
#include "scene/Entity.h"
#include "resource/PipelineManager.h"

namespace trace {
	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	bool ProjectBuilder::BuildProject(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes)
	{
		bool result = true;

		std::filesystem::path out_path(output_dir);
		if (!std::filesystem::exists(out_path) || !std::filesystem::is_empty(out_path))
		{
			TRC_ERROR("Failed to build project, ensure path exists and empty: {}", output_dir);
			return false;
		}

		std::filesystem::create_directory(out_path / "Meta");
		std::filesystem::create_directory(out_path / "Data");
		int bin_id;

		/*
		* appinfo.trbin
		*  '-> Trace_version
		*  '-> build_version
		*  '-> project_name_size
		*  '-> project_name
		*  '-> start_scene
		*/
		// App Info -----------------------------------------------------------------
		FileStream app_info(output_dir + "/Meta/appinfo.trbin", FileMode::WRITE);
		bin_id = TRC_APP_INFO_ID;
		app_info.Write(bin_id); // Binary ID
		int trace_version = TRACE_VERSION;
		app_info.Write(trace_version);
		uint64_t build_version = 0; // TODO: Implement logic to get build version
		app_info.Write(build_version);

		std::string& project_name = project->GetProjectName();
		int name_size = project_name.length() + 1;
		app_info.Write(name_size);
		app_info.Write(project_name.data(), name_size);
		uint64_t start_scene = project->GetStartScene();
		app_info.Write<uint64_t>(start_scene);
		// ------------------------------------------------------------------------------

		/*
		* astdb.trbin
		*  '-> assets db size
		*  '-> assets db data
		*/
		// Assets DB -----------------------------------------------------------------
		FileStream ast_db(output_dir + "/Meta/astdb.trbin", FileMode::WRITE);
		bin_id = TRC_ASSETS_DB_ID;
		ast_db.Write(bin_id);

		std::string assetsDB_path = project->current_directory + "/InternalAssetsDB/assets.trdb";
		FileHandle in_handle;
		if (!FileSystem::open_file(assetsDB_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Failed to build ,Unable to assetsDB file {}", assetsDB_path);
			return false;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		int file_data_size = file_data.length() + 1;
		ast_db.Write(file_data_size);
		ast_db.Write(file_data.data(), file_data_size);

		// ----------------------------------------------------------------------------

		
		return result;
	}

	bool ProjectBuilder::build_project_data(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes)
	{
		int bin_id;
		FileStream scn_bin(output_dir + "/Data/trscn.trbin", FileMode::WRITE);
		bin_id = TRC_SCENE_ID;
		scn_bin.Write(bin_id);
		FileStream tex_bin(output_dir + "/Data/trtex.trbin", FileMode::WRITE);
		bin_id = TRC_TEXTURE_ID;
		tex_bin.Write(bin_id);
		FileStream animc_bin(output_dir + "/Data/tranimc.trbin", FileMode::WRITE);
		bin_id = TRC_ANIMATION_CLIP_ID;
		animc_bin.Write(bin_id);
		FileStream animg_bin(output_dir + "/Data/tranimg.trbin", FileMode::WRITE);
		bin_id = TRC_ANIMATION_GRAPH_ID;
		animg_bin.Write(bin_id);
		FileStream shd_bin(output_dir + "/Data/trshd.trbin", FileMode::WRITE);
		bin_id = TRC_SHADER_ID;
		shd_bin.Write(bin_id);
		FileStream fnt_bin(output_dir + "/Data/trfnt.trbin", FileMode::WRITE);
		bin_id = TRC_FONT_ID;
		fnt_bin.Write(bin_id);
		FileStream mdl_bin(output_dir + "/Data/trmdl.trbin", FileMode::WRITE);
		bin_id = TRC_MODEL_ID;
		mdl_bin.Write(bin_id);
		FileStream mat_bin(output_dir + "/Data/trmat.trbin", FileMode::WRITE);
		bin_id = TRC_MATERIAL_ID;
		mat_bin.Write(bin_id);
		FileStream pip_bin(output_dir + "/Data/trpip.trbin", FileMode::WRITE);
		bin_id = TRC_PIPELINE_ID;
		pip_bin.Write(bin_id);

		std::vector<std::pair<UUID, AssetHeader>> scn_map;
		std::vector<std::pair<UUID, AssetHeader>> tex_map;
		std::vector<std::pair<UUID, AssetHeader>> animc_map;
		std::vector<std::pair<UUID, AssetHeader>> animg_map;
		std::vector<std::pair<UUID, AssetHeader>> shd_map;
		std::vector<std::pair<UUID, AssetHeader>> fnt_map;
		std::vector<std::pair<UUID, AssetHeader>> mdl_map;
		std::vector<std::pair<UUID, AssetHeader>> pip_map;
		std::vector<std::pair<UUID, AssetHeader>> mat_map;

		for (auto& i : scenes)
		{
			FileHandle in_handle;
			if (!FileSystem::open_file(i.string(), FileMode::READ, in_handle))
			{
				TRC_ERROR("Unable to open file {}", i.string());
				continue;
			}
			std::string file_data;
			FileSystem::read_all_lines(in_handle, file_data); // opening file
			FileSystem::close_file(in_handle);

			AssetHeader scn_header;
			scn_header.offset = scn_bin.GetPosition();
			int scn_size = file_data.length() + 1;
			scn_header.data_size = scn_size;

			scn_bin.Write(file_data.data(), scn_size);
			std::pair<UUID, AssetHeader> scn_pair = std::make_pair(GetUUIDFromName(i.filename().string()), scn_header);
			scn_map.push_back(scn_pair);


			SceneSerializer::SerializeTextures(tex_bin, tex_map, file_data);
			SceneSerializer::SerializeAnimationClips(animc_bin, animc_map, file_data);
			SceneSerializer::SerializeAnimationGraphs(animg_bin, animg_map, file_data);
			SceneSerializer::SerializeFonts(fnt_bin, fnt_map, file_data);
			SceneSerializer::SerializeMaterials(mat_bin, mat_map, file_data);
			SceneSerializer::SerializeModels(mdl_bin, mdl_map, file_data);

			SceneSerializer::SerializePipelines(pip_bin, pip_map, file_data);

			SceneSerializer::SerializeShaders(shd_bin, shd_map, file_data);


		}
		PipelineManager::get_instance()->BuildDefaultPipelines(pip_bin, pip_map);
		PipelineManager::get_instance()->BuildDefaultPipelineShaders(shd_bin, shd_map);

		/*
		* buildpck.trbin
		*  '-> assets type count
		*   for each asset type
		*    '-> asset type
		*    '-> asset entry size
		*    '-> asset entry data
		*/
		// Build Pack -------------------------------------------------------
		FileStream pck_db(output_dir + "/Meta/astdb.trbin", FileMode::WRITE);
		bin_id = TRC_ASSETS_DB_ID;
		pck_db.Write(bin_id);

		int assets_type_count = 8;
		pck_db.Write<int>(assets_type_count);

		int map_size = 0;

		map_size = scn_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_SCENE_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(scn_map.data(), map_size);

		map_size = tex_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_TEXTURE_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(tex_map.data(), map_size);
		
		map_size = animc_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_ANIMATION_CLIP_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(animc_map.data(), map_size);

		map_size = animg_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_ANIMATION_GRAPH_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(animg_map.data(), map_size);

		map_size = shd_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_SHADER_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(shd_map.data(), map_size);

		map_size = fnt_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_FONT_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(fnt_map.data(), map_size);

		map_size = mdl_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_MODEL_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(mdl_map.data(), map_size);

		map_size = pip_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_PIPELINE_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(pip_map.data(), map_size);

		map_size = mat_map.size() * sizeof(std::pair<UUID, AssetHeader>);
		bin_id = TRC_MATERIAL_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(mat_map.data(), map_size);


		// ------------------------------------------------------------------

		return true;
	}

}
