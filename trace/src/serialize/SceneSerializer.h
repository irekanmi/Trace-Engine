#pragma once

#include "scene/Scene.h"
#include "resource/Ref.h"
#include "FileStream.h"
#include "AssetsInfo.h"

#include <string>


namespace trace {

	class SceneSerializer
	{

	public:

		static bool Serialize(Ref<Scene> scene, const std::string& file_path);
		static Ref<Scene> Deserialize( const std::string& file_path);

		static bool SerializeTextures(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializeAnimationClips(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializeAnimationGraphs(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializeFonts(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializeModels(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializeMaterials(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializePipelines(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);
		static bool SerializeShaders(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data);

	private:
		
	protected:

	};

}
