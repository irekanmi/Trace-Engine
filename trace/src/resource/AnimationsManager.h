#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "HashTable.h"
#include "animation/Animation.h"
#include "Ref.h"

#include <filesystem>

namespace trace {

	class TRACE_API AnimationsManager
	{

	public:

		bool Init(uint32_t num_clip_units);
		void Shutdown();

		bool LoadDefaults();
		Ref<AnimationClip> LoadClip(const std::string& name);
		Ref<AnimationClip> LoadClip_(const std::string& path);
		Ref<AnimationClip> GetClip(const std::string& name);
		void UnloadClip(AnimationClip* clip);

		static AnimationsManager* get_instance();

	private:
		std::vector<AnimationClip> m_clips;
		HashTable<uint32_t> hashTable;
		uint32_t m_numEntries;

		static AnimationsManager* s_instance;

	protected:

	};

}
