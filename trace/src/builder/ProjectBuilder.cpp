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
#include "resource/PrefabManager.h" 
#include "resource/GenericAssetManager.h"
#include "core/Coretypes.h"
#include "core/Utils.h"
#include "scene/SceneManager.h"
#include "reflection/SerializeTypes.h"

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

		std::unordered_map<UUID, AssetHeader> scn_map;
		std::unordered_map<UUID, AssetHeader> tex_map;
		std::unordered_map<UUID, AssetHeader> animc_map;
		std::unordered_map<UUID, AssetHeader> shd_map;
		std::unordered_map<UUID, AssetHeader> fnt_map;
		std::unordered_map<UUID, AssetHeader> mdl_map;
		std::unordered_map<UUID, AssetHeader> pip_map;
		std::unordered_map<UUID, AssetHeader> mat_map;
		std::unordered_map<UUID, AssetHeader> prefab_map;
		std::unordered_map<UUID, AssetHeader> generic_assets_map;

		// TRC_SCENE_ID
		Reflection::DeserializeContainer(scn_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_TEXTURE_ID
		Reflection::DeserializeContainer(tex_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_ANIMATION_CLIP_ID
		Reflection::DeserializeContainer(animc_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		
		// TRC_SHADER_ID
		Reflection::DeserializeContainer(shd_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_FONT_ID
		Reflection::DeserializeContainer(fnt_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_MODEL_ID
		Reflection::DeserializeContainer(mdl_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_PIPELINE_ID
		Reflection::DeserializeContainer(pip_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_MATERIAL_ID
		Reflection::DeserializeContainer(mat_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		//TRC_PREFAB_ID
		Reflection::DeserializeContainer(prefab_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		//TRC_GENERIC_ASSETS_ID
		Reflection::DeserializeContainer(generic_assets_map, &stream, nullptr, Reflection::SerializationFormat::BINARY);

		SceneManager::get_instance()->SetAssetMap(scn_map);
		TextureManager::get_instance()->SetAssetMap(tex_map);
		AnimationsManager::get_instance()->SetClipsAssetMap(animc_map);
		ShaderManager::get_instance()->SetAssetMap(shd_map);
		FontManager::get_instance()->SetAssetMap(fnt_map);
		ModelManager::get_instance()->SetAssetMap(mdl_map);
		PipelineManager::get_instance()->SetAssetMap(pip_map);
		MaterialManager::get_instance()->SetAssetMap(mat_map);
		PrefabManager::get_instance()->SetAssetMap(prefab_map);
		GenericAssetManager::get_instance()->SetAssetMap(generic_assets_map);


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
		FileStream prefab_bin(output_dir + "/Data/trprf.trbin", FileMode::WRITE);
		bin_id = TRC_PREFAB_ID;
		prefab_bin.Write(bin_id);
		FileStream generic_bin(output_dir + "/Data/generic.trbin", FileMode::WRITE);
		bin_id = TRC_GENERIC_ASSETS_ID;
		generic_bin.Write(bin_id);

		std::unordered_map<UUID, AssetHeader> scn_map;
		std::unordered_map<UUID, AssetHeader> tex_map;
		std::unordered_map<UUID, AssetHeader> animc_map;
		std::unordered_map<UUID, AssetHeader> shd_map;
		std::unordered_map<UUID, AssetHeader> fnt_map;
		std::unordered_map<UUID, AssetHeader> mdl_map;
		std::unordered_map<UUID, AssetHeader> pip_map;
		std::unordered_map<UUID, AssetHeader> mat_map;
		std::unordered_map<UUID, AssetHeader> prefab_map;
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
			scn_header.offset = scn_bin.GetPosition();
			SceneSerializer::Serialize(current_scene, &scn_bin);
			scn_header.data_size = scn_bin.GetPosition() - scn_header.offset;
			std::pair<UUID, AssetHeader> scn_pair = std::make_pair(GetUUIDFromName(current_scene->GetName()), scn_header);
			scn_map.emplace(scn_pair);


			SceneSerializer::SerializeTextures(tex_bin, tex_map, current_scene);
			SceneSerializer::SerializeAnimationClips(animc_bin, animc_map, current_scene);
			SceneSerializer::SerializeFonts(fnt_bin, fnt_map, current_scene);
			SceneSerializer::SerializeMaterials(mat_bin, mat_map, current_scene);
			SceneSerializer::SerializeModels(mdl_bin, mdl_map, current_scene);
			SceneSerializer::SerializePipelines(pip_bin, pip_map, current_scene);
			SceneSerializer::SerializeShaders(shd_bin, shd_map, current_scene);
			SceneSerializer::SerializePrefabs(prefab_bin, prefab_map, current_scene);
			SceneSerializer::SerializeAnimationGraphs(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeSkeletons(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeSequences(generic_bin, generic_assets_map, current_scene);
			SceneSerializer::SerializeSkinnedModels(generic_bin, generic_assets_map, current_scene);


		}
		PipelineManager::get_instance()->BuildDefaultPipelines(pip_bin, pip_map);
		PipelineManager::get_instance()->BuildDefaultPipelineShaders(shd_bin, shd_map);
		ModelManager::get_instance()->BuildDefaultModels(mdl_bin, mdl_map);

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


		// TRC_SCENE_ID
		Reflection::SerializeContainer(scn_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_TEXTURE_ID
		Reflection::SerializeContainer(tex_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_ANIMATION_CLIP_ID
		Reflection::SerializeContainer(animc_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		
		// TRC_SHADER_ID
		Reflection::SerializeContainer(shd_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_FONT_ID
		Reflection::SerializeContainer(fnt_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_MODEL_ID
		Reflection::SerializeContainer(mdl_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_PIPELINE_ID
		Reflection::SerializeContainer(pip_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		// TRC_MATERIAL_ID
		Reflection::SerializeContainer(mat_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		//TRC_PREFAB_ID
		Reflection::SerializeContainer(prefab_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);

		//TRC_GENERIC_ASSETS_ID
		Reflection::SerializeContainer(generic_assets_map, &pck_db, nullptr, Reflection::SerializationFormat::BINARY);


		// ------------------------------------------------------------------

		return true;
	}

}
