#include "pch.h"

#include "ProjectBuilder.h"
#include "serialize/FileStream.h"
#include "serialize/AssetsInfo.h"
#include "serialize/SceneSerializer.h"
#include "scene/Entity.h" 
#include "resource/PrefabManager.h" 
#include "resource/GenericAssetManager.h"
#include "core/Coretypes.h"
#include "core/Utils.h"
#include "reflection/SerializeTypes.h"
#include "external_utils.h"
#include "resource/DefaultAssetsManager.h"

#include "serialize/yaml_util.h"

namespace trace {

	bool ProjectBuilder::BuildProject(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes)
	{
		bool result = true;

		std::filesystem::path out_path(output_dir);
		if (!std::filesystem::exists(out_path) /*|| !std::filesystem::is_empty(out_path)*/)
		{
			TRC_ERROR("Failed to build project, ensure path exists and empty: {}", output_dir);
			return false;
		}

		if (!std::filesystem::exists(out_path / "Meta"))
		{
			std::filesystem::create_directory(out_path / "Meta");
		}
		if (!std::filesystem::exists(out_path / "Data"))
		{
			std::filesystem::create_directory(out_path / "Data");
		}
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
		int32_t name_size = static_cast<int32_t>(project_name.size());
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

		int32_t file_data_size = static_cast<int32_t>(file_data.size());
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

		std::unordered_map<UUID, AssetHeader> generic_assets_map;

		//TRC_GENERIC_ASSETS_ID
		Reflection::DeserializeContainer(generic_assets_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		GenericAssetManager::get_instance()->SetAssetMap(generic_assets_map);


		return true;
	}

	bool ProjectBuilder::build_project_data(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes)
	{
		int bin_id;
		FileStream generic_bin(output_dir + "/Data/generic.trbin", FileMode::WRITE);
		bin_id = TRC_GENERIC_ASSETS_ID;
		generic_bin.Write(bin_id);

		std::unordered_map<UUID, AssetHeader> generic_assets_map;



		for (auto& i : scenes)
		{

			Ref<Scene> current_scene = SceneSerializer::Deserialize(i.string());
			if (!current_scene)
			{
				TRC_ERROR("Unable load scene {}", i.string());
				continue;
			}
			AssetHeader scn_header;
			scn_header.offset = generic_bin.GetPosition();
			SceneSerializer::Serialize(current_scene, &generic_bin);
			scn_header.data_size = generic_bin.GetPosition() - scn_header.offset;
			std::pair<UUID, AssetHeader> scn_pair = std::make_pair(GetUUIDFromName(current_scene->GetName()), scn_header);
			generic_assets_map.emplace(scn_pair);


			SceneSerializer::SerializeTextures(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeAnimationClips(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeFonts(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeMaterials(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeModels(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializePipelines(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeShaders(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializePrefabs(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeAnimationGraphs(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeSkeletons(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeSequences(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeSkinnedModels(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeShaderGraphs(generic_bin, generic_assets_map, current_scene);


		}

		DefaultAssetsManager::BuildDefaults(generic_bin, generic_assets_map);
		GenericAssetManager::get_instance()->BuildPipeline(generic_bin, generic_assets_map);
		
		


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

		//TRC_GENERIC_ASSETS_ID
		Reflection::SerializeContainer(generic_assets_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);


		// ------------------------------------------------------------------

		return true;
	}

}
