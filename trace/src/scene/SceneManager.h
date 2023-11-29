#pragma once

#include "Entity.h"
#include "Scene.h"
#include "resource/Ref.h"
#include "resource/HashTable.h"

namespace trace {

	class SceneManager
	{

	public:

		bool Init(uint32_t num_entries);
		void Shutdown();

		Ref<Scene> CreateScene(const std::string& name);
		Ref<Scene> GetScene(const std::string& name);
		Ref<Scene> CreateScene_(const std::string& file_path);

		void UnloadScene(Scene* scene);

		static SceneManager* get_instance();
	private:
		std::vector<Scene> m_scenes;
		HashTable<uint32_t> m_hashTable;
		uint32_t m_entries = 0;

		static SceneManager* s_instance;
	protected:

	};

}
