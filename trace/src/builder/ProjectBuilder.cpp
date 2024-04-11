#include "pch.h"

#include "ProjectBuilder.h"
#include "serialize/FileStream.h"
#include "serialize/AssetsInfo.h"
#include "serialize/SceneSerializer.h"
#include "scene/Entity.h"

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
		// ------------------------------------------------------------------------------


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

		std::vector<std::pair<UUID, AssetHeader>> scn_map;
		std::vector<std::pair<UUID, AssetHeader>> tex_map;
		std::vector<std::pair<UUID, AssetHeader>> animc_map;
		std::vector<std::pair<UUID, AssetHeader>> animg_map;

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

		}

		return true;
	}

}
