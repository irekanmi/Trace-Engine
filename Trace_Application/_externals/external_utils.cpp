
#include "render/GTexture.h"
#include "resource/Ref.h"
#include "scene/UUID.h"
#include "render/Model.h"
#include "render/SkinnedModel.h"

#include "../src/TraceGame.h"

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

	Ref<Model> LoadModel(UUID uuid, std::string& model_name)
	{
		return Ref<Model>();
	}

	Ref<SkinnedModel> LoadSkinnedModel(UUID uuid, std::string& model_name)
	{
		return Ref<SkinnedModel>();
	}

	Ref<GTexture> LoadTexture(UUID uuid)
	{
		return Ref<GTexture>();
	}

	bool LoadAndSetScene(const std::string& filename)
	{

		Ref<Scene> scene = SceneManager::get_instance()->GetScene(filename);
		if (!scene)
		{
			UUID id = GetUUIDFromName(filename);
			std::string file_path = GetPathFromUUID(id).string();
			scene = SceneManager::get_instance()->LoadScene_Runtime(id);
			if (!scene)
			{
				TRC_ERROR("Unable to load scene, Filename:{} , Function: {}", filename, __FUNCTION__);
				return false;
			}
		}

		TraceGame* game = TraceGame::get_instance();

		game->SetNextScene(scene);


		return true;
	}

}
