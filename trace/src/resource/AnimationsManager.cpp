#include "pch.h"

#include "AnimationsManager.h"
#include "serialize/FileStream.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Utils.h"
#include "core/Coretypes.h"


namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);

	AnimationsManager* AnimationsManager::s_instance = nullptr;

	bool AnimationsManager::Init(uint32_t num_clip_units, uint32_t num_graph_units)
	{
		// Animations Clips ---------------------------------
		m_numEntries = num_clip_units;

		m_clips.resize(m_numEntries);
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_clips[i].m_id = INVALID_ID;
		}

		hashTable.Init(m_numEntries);
		hashTable.Fill(INVALID_ID);
		// ---------------------------------------------------

		// Animation Graphs ---------------------------------
		m_numGraphEntries = num_graph_units;

		m_graphs.resize(m_numGraphEntries);
		for (uint32_t i = 0; i < m_numGraphEntries; i++)
		{
			m_graphs[i].m_id = INVALID_ID;
		}

		graphHashTable.Init(m_numEntries);
		graphHashTable.Fill(INVALID_ID);
		// ---------------------------------------------------

		return true;
	}

	void AnimationsManager::Shutdown()
	{
		// Animations Clips -----------------------------
		if (!m_clips.empty())
		{
			for (AnimationClip& clip : m_clips)
			{
				if (clip.m_id == INVALID_ID)
					continue;
				TRC_DEBUG("Unloaded Animation Clip, name:{}, RefCount : {}", clip.GetName(), clip.m_refCount);
				UnloadClip(&clip);
			}
			m_clips.clear();
		}
		// ------------------------------------------------

		// Animation Graphs ------------------------------
		if (!m_graphs.empty())
		{
			for (AnimationGraph& graph : m_graphs)
			{
				if (graph.m_id == INVALID_ID)
					continue;
				TRC_DEBUG("Unloaded Animation Graph, name:{}, RefCount : {}", graph.GetName(), graph.m_refCount);
				UnloadGraph(&graph);
			}
			m_graphs.clear();
		}
		// -----------------------------------------------
	}

	bool AnimationsManager::LoadDefaults()
	{
		return true;
	}

	Ref<AnimationClip> AnimationsManager::CreateClip(const std::string& name)
	{
		Ref<AnimationClip> result;
		AnimationClip* _clip = nullptr;
		result = GetClip(name);
		if (result)
		{
			TRC_WARN("Material {} already exists", name);
			return result;
		}


		uint32_t i = 0;
		for (AnimationClip& clip : m_clips)
		{
			if (clip.m_id == INVALID_ID)
			{
				clip.GetTracks().clear();
				clip.SetDuration(1.0f);
				clip.SetSampleRate(30);
				clip.m_id = i;
				hashTable.Set(name, i);
				_clip = &clip;
				_clip->m_path = name;
				
				break;
			}
			i++;
		}



		result = { _clip, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadClip) };
		return result;
	}

	Ref<AnimationClip> AnimationsManager::LoadClip(const std::string& name)
	{
		return Ref<AnimationClip>();
	}

	Ref<AnimationClip> AnimationsManager::LoadClip_(const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		Ref<AnimationClip> result = GetClip(name);
		AnimationClip* _clip = nullptr;
		uint32_t hash = hashTable.Get(name);
		if (result)
		{
			TRC_WARN("{} animation clip has been loaded", name);
			return result;
		}

		uint32_t i = 0;
		for (AnimationClip& clip : m_clips)
		{
			if (clip.m_id == INVALID_ID)
			{
				// TODO: Implement Loading of Animation Clips
				clip.SetDuration(1.0f);
				clip.SetSampleRate(30);
				clip.m_id = i;
				hashTable.Set(name, i);
				_clip = &clip;
				_clip->m_path = p;
				break;
			}

			i++;
		}

		result = { _clip, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadClip) };
		return result;
	}

	Ref<AnimationClip> AnimationsManager::GetClip(const std::string& name)
	{
		Ref<AnimationClip> result;
		AnimationClip* _clip = nullptr;
		uint32_t hash = hashTable.Get(name);
		if (hash == INVALID_ID)
		{
			TRC_WARN("{} animation clip has not been loaded", name);
			return result;
		}
		_clip = &m_clips[hash];
		if (_clip->m_id == INVALID_ID)
		{
			TRC_WARN("{} animation clip has been destroyed", name);
			hash = INVALID_ID;
			return result;
		}

		result = { _clip, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadClip) };
		return result;
	}

	void AnimationsManager::UnloadClip(AnimationClip* clip)
	{
		
		clip->m_id = INVALID_ID;
		clip->GetTracks().clear();
	}

	Ref<AnimationClip> AnimationsManager::LoadClip_Runtime(UUID id)
	{
		Ref<AnimationClip> result;
		auto it = m_clipAssetMap.find(id);
		if (it == m_clipAssetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		AnimationClip* _clip = nullptr;
		std::string name = GetNameFromUUID(id);

		result = GetClip(name);
		if (result) return result;

		uint32_t i = 0;
		for (AnimationClip& clip : m_clips)
		{
			if (clip.m_id == INVALID_ID)
			{
				// TODO: Implement Loading of Animation Clips
				clip.SetDuration(1.0f);
				clip.SetSampleRate(30);
				clip.m_id = i;
				hashTable.Set(name, i);
				_clip = &clip;
				break;
			}

			i++;
		}

		result = { _clip, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadClip) };

		std::string bin_dir;
		FindDirectory(AppSettings::exe_path, "Data/tranimc.trbin", bin_dir);
		FileStream stream(bin_dir, FileMode::READ);
		AnimationsSerializer::DeserializeAnimationClip(result, stream, it->second);

		return result;
	}

	Ref<AnimationGraph> AnimationsManager::CreateGraph(const std::string& name)
	{
		Ref<AnimationGraph> result;
		AnimationGraph* _graph = nullptr;
		result = GetGraph(name);
		if (result)
		{
			TRC_WARN("Material {} already exists", name);
			return result;
		}


		uint32_t i = 0;
		for (AnimationGraph& graph : m_graphs)
		{
			if (graph.m_id == INVALID_ID)
			{
				graph.GetStates().clear();
				graph.currrent_state_index = 0;
				graph.m_id = i;
				graphHashTable.Set(name, i);
				_graph = &graph;
				_graph->m_path = name;
				break;
			}
			i++;
		}



		result = { _graph, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadGraph) };
		return result;
	}

	Ref<AnimationGraph> AnimationsManager::LoadGraph(const std::string& name)
	{
		return Ref<AnimationGraph>();
	}

	Ref<AnimationGraph> AnimationsManager::LoadGraph_(const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		Ref<AnimationGraph> result = GetGraph(name);
		AnimationGraph* _graph = nullptr;
		uint32_t hash = hashTable.Get(name);
		if (result)
		{
			TRC_WARN("{} animation graph has been loaded", name);
			return result;
		}

		uint32_t i = 0;
		for (AnimationGraph& graph : m_graphs)
		{
			if (graph.m_id == INVALID_ID)
			{
				// TODO: Implement Loading of Animation Graphs
				graph.GetStates().clear();
				graph.currrent_state_index = -1;
				graph.m_id = i;
				graphHashTable.Set(name, i);
				_graph = &graph;
				_graph->m_path = p;
				break;
			}

			i++;
		}

		result = { _graph, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadGraph) };
		return result;
	}

	Ref<AnimationGraph> AnimationsManager::GetGraph(const std::string& name)
	{
		Ref<AnimationGraph> result;
		AnimationGraph* _graph = nullptr;
		uint32_t hash = graphHashTable.Get(name);
		if (hash == INVALID_ID)
		{
			TRC_WARN("{} animation graph has not been loaded", name);
			return result;
		}
		_graph = &m_graphs[hash];
		if (_graph->m_id == INVALID_ID)
		{
			TRC_WARN("{} animation graph has been destroyed", name);
			hash = INVALID_ID;
			return result;
		}

		result = { _graph, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadGraph) };
		return result;
	}

	void AnimationsManager::UnloadGraph(AnimationGraph* graph)
	{
		if (graph->m_refCount > 0)
		{
			TRC_WARN("{} Graph is still in use", graph->GetName());
			return;
		}

		graph->m_id = INVALID_ID;
		graph->GetStates().clear();
	}

	Ref<AnimationGraph> AnimationsManager::LoadGraph_Runtime(UUID id)
	{
		Ref<AnimationGraph> result;
		auto it = m_graphAssetMap.find(id);
		if (it == m_graphAssetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		AnimationGraph* _graph = nullptr;
		std::string name = GetNameFromUUID(id);

		result = GetGraph(name);
		if (result) return result;

		uint32_t i = 0;
		for (AnimationGraph& graph : m_graphs)
		{
			if (graph.m_id == INVALID_ID)
			{
				// TODO: Implement Loading of Animation Graphs
				graph.GetStates().clear();
				graph.currrent_state_index = -1;
				graph.m_id = i;
				graphHashTable.Set(name, i);
				_graph = &graph;
				break;
			}

			i++;
		}

		result = { _graph, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadGraph) };

		std::string bin_dir;
		FindDirectory(AppSettings::exe_path, "Data/tranimg.trbin", bin_dir);
		FileStream stream(bin_dir, FileMode::READ);
		AnimationsSerializer::DeserializeAnimationGraph(result, stream, it->second);

		return result;
	}

	AnimationsManager* AnimationsManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new AnimationsManager;
		}
		return s_instance;
	}

}