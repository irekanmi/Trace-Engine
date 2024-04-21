#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "HashTable.h"
#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
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
		void UnloadClip(AnimationClip* clip);
		Ref<AnimationClip> LoadClip_Runtime(UUID id);

		Ref<AnimationGraph> CreateGraph(const std::string& name);
		Ref<AnimationGraph> LoadGraph(const std::string& name);
		Ref<AnimationGraph> LoadGraph_(const std::string& path);
		Ref<AnimationGraph> GetGraph(const std::string& name);
		void UnloadGraph(AnimationGraph* graph);
		Ref<AnimationGraph> LoadGraph_Runtime(UUID id);

		void SetClipsAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_clipAssetMap = map;
		}

		void SetGraphsAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_graphAssetMap = map;
		}

		static AnimationsManager* get_instance();

	private:
		std::vector<AnimationClip> m_clips;
		std::vector<AnimationGraph> m_graphs;
		HashTable<uint32_t> hashTable;
		HashTable<uint32_t> graphHashTable;
		uint32_t m_numEntries;
		uint32_t m_numGraphEntries;
		std::unordered_map<UUID, AssetHeader> m_clipAssetMap;
		std::unordered_map<UUID, AssetHeader> m_graphAssetMap;

		static AnimationsManager* s_instance;

	protected:

	};

}
