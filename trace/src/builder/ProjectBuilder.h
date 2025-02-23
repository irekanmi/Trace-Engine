#pragma once

#include "resource/Ref.h"
#include "project/Project.h"
#include "core/Enums.h"

#include <string>

#define TRC_APP_INFO_ID MAKE_VERSION(1, 0, 0, 0)
#define TRC_ASSETS_DB_ID MAKE_VERSION(2, 0, 0, 0)
#define TRC_SHADER_ID MAKE_VERSION(3, 0, 0, 0)
#define TRC_ANIMATION_CLIP_ID MAKE_VERSION(4, 0, 0, 0)
#define TRC_SCENE_ID MAKE_VERSION(5, 0, 0, 0)
#define TRC_TEXTURE_ID MAKE_VERSION(6, 0, 0, 0)
#define TRC_ANIMATION_GRAPH_ID MAKE_VERSION(7, 0, 0, 0)
#define TRC_FONT_ID MAKE_VERSION(8, 0, 0, 0)
#define TRC_MATERIAL_ID MAKE_VERSION(9, 0, 0, 0)
#define TRC_MODEL_ID MAKE_VERSION(10, 0, 0, 0)
#define TRC_PIPELINE_ID MAKE_VERSION(11, 0, 0, 0)
#define TRC_BUILD_PACK_ID MAKE_VERSION(12, 0, 0, 0)
#define TRC_ANIM_GRAPH_ID MAKE_VERSION(13, 0, 0, 0)
#define TRC_SKELETON_ID MAKE_VERSION(14, 0, 0, 0)
#define TRC_PREFAB_ID MAKE_VERSION(15, 0, 0, 0)
#define TRC_ANIMATION_SEQUENCE_ID MAKE_VERSION(16, 0, 0, 0)
#define TRC_SKINNED_MODEL_ID MAKE_VERSION(17, 0, 0, 0)
#define TRC_GENERIC_ASSETS_ID MAKE_VERSION(18, 0, 0, 0)

namespace trace {

	class ProjectBuilder
	{

	public:

		static bool BuildProject(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes);
		static bool LoadAssetsDB(std::unordered_map<std::string, UUID>& file_ids, std::unordered_map<UUID, std::string>& id_names);
		static bool LoadBuildPack();

	private:
		static bool build_project_data(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes);

	protected:

	};

}
