#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "HashTable.h"
#include "animation/Animation.h"
#include "Ref.h"
#include "serialize/AssetsInfo.h"

#include <filesystem>

namespace trace {

	class TRACE_API AnimationsManager
	{

	public:

		bool Init(uint32_t num_clip_units, uint32_t num_graph_units);
		void Shutdown();

		bool LoadDefaults();
		Ref<AnimationClip> CreateClip(const std::string& name);
		Ref<AnimationClip> LoadClip(const std::string& name);
		Ref<AnimationClip> LoadClip_(const std::string& path);
		Ref<AnimationClip> GetClip(const std::string& name);
		void UnloadClip(Resource* clip);
		Ref<AnimationClip> LoadClip_Runtime(UUID id);


		void SetClipsAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_clipAssetMap = map;
		}


		void RenameAsset(Ref<AnimationClip> asset, const std::string& new_name);

		static AnimationsManager* get_instance();

	private:
		std::vector<AnimationClip> m_clips;
		HashTable<uint32_t> hashTable;
		uint32_t m_numEntries;
		std::unordered_map<UUID, AssetHeader> m_clipAssetMap;


	protected:

	};

}
