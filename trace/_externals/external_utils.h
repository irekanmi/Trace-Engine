#pragma once

#include "scene/UUID.h"
#include "serialize/yaml_util.h"
#include "render/GTexture.h"
#include "resource/Ref.h"
#include "scene/Components.h"


#include <filesystem>
#include <string>

namespace trace {

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);
	extern std::string GetNameFromUUID(UUID uuid);
	extern bool LoadModel(UUID uuid, ModelComponent& out_model, YAML::Node& node);
	extern Ref<GTexture> LoadTexture(UUID uuid);

}