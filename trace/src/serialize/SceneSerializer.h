#pragma once

#include "scene/Scene.h"
#include "resource/Ref.h"
#include "FileStream.h"
#include "AssetsInfo.h"
#include "resource/Prefab.h"

#include <string>


namespace trace {

	class SceneSerializer
	{

	public:

		static bool Serialize(Ref<Scene> scene, const std::string& file_path);
		static bool SerializePrefab(Ref<Prefab> prefab, const std::string& file_path);
		static bool SerializePrefab(Ref<Prefab> prefab, DataStream* stream);
		static Ref<Scene> Deserialize( const std::string& file_path);
		static Ref<Prefab> DeserializePrefab( const std::string& file_path);
		static Ref<Prefab> DeserializePrefab( DataStream* stream);
		static bool Deserialize(Ref<Scene> scene, FileStream& stream, AssetHeader& header);
		static bool Serialize(Ref<Scene> scene, DataStream* stream);
		static Ref<Scene> Deserialize(DataStream* stream);

		static bool SerializeTextures(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeAnimationClips(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeAnimationGraphs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeFonts(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeModels(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeMaterials(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeShaderGraphs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializePipelines(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeShaders(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializePrefabs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeSkeletons(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeSequences(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);
		static bool SerializeSkinnedModels(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene);

	private:
		
	protected:

	};

}
