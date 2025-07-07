#include "pch.h"

#include "AnimationsManager.h"
#include "serialize/FileStream.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Utils.h"
#include "core/Coretypes.h"


namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);


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

	void AnimationsManager::UnloadClip(Resource* res)
	{
		AnimationClip* clip = (AnimationClip*)res;

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

		std::string bin_dir;
		FindDirectory(AppSettings::exe_path, "Data/tranimc.trbin", bin_dir);
		FileStream stream(bin_dir, FileMode::READ);
		stream.SetPosition(it->second.offset);
		result = AnimationsSerializer::DeserializeAnimationClip(&stream);

		return result;
	}


	void AnimationsManager::RenameAsset(Ref<AnimationClip> asset, const std::string& new_name)
	{
		uint32_t index = hashTable.Get(asset->GetName());
		hashTable.Set(asset->GetName(), INVALID_ID);
		hashTable.Set(new_name, index);
	}

	AnimationsManager* AnimationsManager::get_instance()
	{
		static AnimationsManager* s_instance = new AnimationsManager;
		return s_instance;
	}

}