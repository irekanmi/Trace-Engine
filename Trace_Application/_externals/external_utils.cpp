
#include "../src/TraceGame.h"
#include "serialize/yaml_util.h"
#include "render/GTexture.h"
#include "resource/Ref.h"

namespace trace {

	extern std::unordered_map<std::string, UUID> file_ids;
	extern std::unordered_map<UUID, std::string> id_names;

	std::filesystem::path GetPathFromUUID(UUID uuid)
	{
		return "";
	}
	UUID GetUUIDFromName(const std::string& name)
	{
		return file_ids[name];
	}

	std::string GetNameFromUUID(UUID uuid)
	{
		return id_names[uuid];
	}

	bool LoadModel(UUID uuid, ModelComponent& out_model, YAML::Node& node)
	{
		return false;
	}

	Ref<GTexture> LoadTexture(UUID uuid)
	{
		return Ref<GTexture>();
	}

}
