#pragma once

#include "scene/UUID.h"
#include "serialize/yaml_util.h"
#include "render/GTexture.h"
#include "resource/Ref.h"
#include "render/SkinnedModel.h"
#include "render/Model.h"


#include <filesystem>
#include <string>

namespace trace {

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);
	extern std::string GetNameFromUUID(UUID uuid);
	extern Ref<Model> LoadModel(UUID uuid, std::string& model_name);
	extern Ref<SkinnedModel> LoadSkinnedModel(UUID uuid, std::string& model_name);
	extern Ref<GTexture> LoadTexture(UUID uuid);

}