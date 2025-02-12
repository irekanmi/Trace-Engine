#include "pch.h"

#include "ProjectBuilder.h"
#include "serialize/FileStream.h"
#include "serialize/AssetsInfo.h"
#include "serialize/SceneSerializer.h"
#include "scene/Entity.h"
#include "resource/PipelineManager.h"
#include "resource/AnimationsManager.h" 
#include "resource/FontManager.h" 
#include "resource/MaterialManager.h" 
#include "resource/MeshManager.h" 
#include "resource/ModelManager.h" 
#include "resource/ShaderManager.h" 
#include "resource/TextureManager.h" 
#include "core/Coretypes.h"
#include "core/Utils.h"
#include "scene/SceneManager.h"

#include "serialize/yaml_util.h"

namespace trace {
	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	bool ProjectBuilder::BuildProject(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes)
	{
		bool result = true;

		std::filesystem::path out_path(output_dir);
		if (!std::filesystem::exists(out_path) /*|| !std::filesystem::is_empty(out_path)*/)
		{
			TRC_ERROR("Failed to build project, ensure path exists and empty: {}", output_dir);
			return false;
		}

		if (!std::filesystem::exists(out_path / "Meta")) std::filesystem::create_directory(out_path / "Meta");
		if (!std::filesystem::exists(out_path / "Data")) std::filesystem::create_directory(out_path / "Data");
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
		int32_t name_size = static_cast<int32_t>(project_name.length() + 1);
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

		std::string assetsDB_path = project->GetProjectCurrentDirectory() + "/InternalAssetsDB/assets.trdb";
		FileHandle in_handle;
		if (!FileSystem::open_file(assetsDB_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Failed to build ,Unable to assetsDB file {}", assetsDB_path);
			return false;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		int32_t file_data_size = static_cast<int32_t>(file_data.length() + 1);
		ast_db.Write(file_data_size);
		ast_db.Write(file_data.data(), file_data_size);

		// ----------------------------------------------------------------------------

		build_project_data(project, output_dir, scenes);
		
		return result;
	}

	bool ProjectBuilder::LoadAssetsDB(std::unordered_map<std::string, UUID>& file_ids, std::unordered_map<UUID, std::string>& id_names)
	{
		std::string astdb_path;
		FindDirectory(AppSettings::exe_path, "Meta/astdb.trbin", astdb_path);

		FileStream stream(astdb_path, FileMode::READ);
		int bin_id = 0;
		stream.Read<int>(bin_id);
		if (bin_id != (int)(TRC_ASSETS_DB_ID))
		{
			TRC_ERROR("file is not a valid app info data, path -> {}", astdb_path);
			return false;
		}

		int db_size = 0;
		stream.Read<int>(db_size);
		char* data = new char[db_size];// TODO: Use custom allocator
		stream.Read(data, db_size);
		std::string db_data = data;
		delete[] data;// TODO: Use custom allocator

		YAML::Node _data = YAML::Load(db_data);

		YAML::Node DATA = _data["DATA"];
		for (auto i : DATA)
		{
			std::string filename = i["Name"].as<std::string>();
			UUID id = i["UUID"].as<uint64_t>();
			file_ids[filename] = id;
			id_names[id] = filename;
		}

		return true;
	}

	bool ProjectBuilder::LoadBuildPack()
	{
		std::string pack_path;
		FindDirectory(AppSettings::exe_path, "Meta/buildpck.trbin", pack_path);

		FileStream stream(pack_path, FileMode::READ);
		int bin_id = 0;
		stream.Read<int>(bin_id);
		if (bin_id != (int)(TRC_BUILD_PACK_ID))
		{
			TRC_ERROR("file is not a valid app info data, path -> {}", pack_path);
			return false;
		}

		int asset_type_count = 0;
		stream.Read<int>(asset_type_count);

		for (int i = 0; i < asset_type_count; i++)
		{
			int asset_type = 0;
			stream.Read<int>(asset_type);
			std::unordered_map<UUID, AssetHeader> map;
			std::vector<std::pair<UUID, AssetHeader>> arr_map;

			int map_size = 0;
			stream.Read<int>(map_size);
			arr_map.resize(map_size / sizeof(std::pair<UUID, AssetHeader>));
			stream.Read(arr_map.data(), map_size);

			for (auto& ast_h : arr_map)
			{
				map.emplace(ast_h);
			}


			switch (asset_type)
			{
			case TRC_SCENE_ID:
			{				
				SceneManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_SHADER_ID:
			{				
				ShaderManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_TEXTURE_ID:
			{				
				TextureManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_FONT_ID:
			{				
				FontManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_MATERIAL_ID:
			{				
				MaterialManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_MODEL_ID:
			{				
				ModelManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_PIPELINE_ID:
			{				
				PipelineManager::get_instance()->SetAssetMap(map);
				break;
			}
			case TRC_ANIMATION_CLIP_ID:
			{				
				AnimationsManager::get_instance()->SetClipsAssetMap(map);
				break;
			}
			case TRC_ANIMATION_GRAPH_ID:
			{				
				AnimationsManager::get_instance()->SetGraphsAssetMap(map);
				break;
			}
			}

			
		}

		

		return true;
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
			int32_t scn_size = static_cast<int32_t>(file_data.length() + 1);

			scn_bin.Write(file_data.data(), scn_size);
			scn_header.data_size = scn_bin.GetPosition() - scn_header.offset;
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
		FileStream pck_db(output_dir + "/Meta/buildpck.trbin", FileMode::WRITE);
		bin_id = TRC_BUILD_PACK_ID;
		pck_db.Write(bin_id);

		int32_t assets_type_count = 9;
		pck_db.Write<int32_t>(assets_type_count);

		int32_t map_size = 0;

		map_size = static_cast<int32_t>(scn_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_SCENE_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(scn_map.data(), map_size);

		map_size = static_cast<int32_t>(tex_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_TEXTURE_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(tex_map.data(), map_size);
		
		map_size = static_cast<int32_t>(animc_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_ANIMATION_CLIP_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(animc_map.data(), map_size);

		map_size = static_cast<int32_t>(animg_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_ANIMATION_GRAPH_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(animg_map.data(), map_size);

		map_size = static_cast<int32_t>(shd_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_SHADER_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(shd_map.data(), map_size);

		map_size = static_cast<int32_t>(fnt_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_FONT_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(fnt_map.data(), map_size);

		map_size = static_cast<int32_t>(mdl_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_MODEL_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(mdl_map.data(), map_size);

		map_size = static_cast<int32_t>(pip_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_PIPELINE_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(pip_map.data(), map_size);

		map_size = static_cast<int32_t>(mat_map.size() * sizeof(std::pair<UUID, AssetHeader>));
		bin_id = TRC_MATERIAL_ID;
		pck_db.Write(bin_id);
		pck_db.Write(map_size);
		pck_db.Write(mat_map.data(), map_size);


		// ------------------------------------------------------------------

		return true;
	}

}
