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

namespace trace {

	class ProjectBuilder
	{

	public:

		static bool BuildProject(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes);

	private:
		static bool build_project_data(Ref<Project> project, std::string output_dir, std::unordered_set<std::filesystem::path>& scenes);

	protected:

	};

}
