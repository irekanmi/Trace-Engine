#include "pch.h"

#include "AnimationsManager.h"

namespace trace {


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
				clip.SetDuration(0.0f);
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
		Ref<AnimationClip> result;
		AnimationClip* _clip = nullptr;
		uint32_t hash = hashTable.Get(name);
		if (hash != INVALID_ID)
		{
			TRC_WARN("{} animation clip has been loaded", name);
			return GetClip(name);
		}

		uint32_t i = 0;
		for (AnimationClip& clip : m_clips)
		{
			if (clip.m_id == INVALID_ID)
			{
				// TODO: Implement Loading of Animation Clips
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
		result = { _clip, BIND_RENDER_COMMAND_FN(AnimationsManager::UnloadClip) };
		return result;
	}

	void AnimationsManager::UnloadClip(AnimationClip* clip)
	{
		if (clip->m_refCount > 0)
		{
			TRC_WARN("{} is still in use", clip->GetName());
			return;
		}

		clip->m_id = INVALID_ID;
		clip->~AnimationClip(); // TODO: Find a better way to destroy the animation clip
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
		Ref<AnimationGraph> result;
		AnimationGraph* _graph = nullptr;
		uint32_t hash = hashTable.Get(name);
		if (hash != INVALID_ID)
		{
			TRC_WARN("{} animation graph has been loaded", name);
			return GetGraph(name);
		}

		uint32_t i = 0;
		for (AnimationGraph& graph : m_graphs)
		{
			if (graph.m_id == INVALID_ID)
			{
				// TODO: Implement Loading of Animation Graphs
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
		graph->~AnimationGraph(); // TODO: Find a better way to destroy the animation graph
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