#include "pch.h"

#include "AnimationsManager.h"

namespace trace {


	AnimationsManager* AnimationsManager::s_instance = nullptr;

	bool AnimationsManager::Init(uint32_t num_clip_units)
	{
		m_numEntries = num_clip_units;

		m_clips.resize(m_numEntries);
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_clips[i].m_id = INVALID_ID;
		}

		hashTable.Init(m_numEntries);
		hashTable.Fill(INVALID_ID);

		return true;
	}

	void AnimationsManager::Shutdown()
	{
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

	}

	bool AnimationsManager::LoadDefaults()
	{
		return true;
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
			TRC_WARN("{} font has been loaded", name);
			return GetClip(name);
		}

		uint32_t i = 0;
		for (AnimationClip& clip : m_clips)
		{
			if (clip.m_id == INVALID_ID)
			{
				std::string file_path = path;
				if (false)// TODO: Implement Loading of Animation Clips
				{
					TRC_ERROR("Failed to load to font, path->{}", file_path);
					return result;
				}
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
			TRC_WARN("{} font has not been loaded", name);
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

	AnimationsManager* AnimationsManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new AnimationsManager;
		}
		return s_instance;
	}

}