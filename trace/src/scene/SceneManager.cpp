#include "pch.h"

#include "SceneManager.h"
#include "core/io/Logging.h"
#include "core/Enums.h"

namespace trace {

	SceneManager* SceneManager::s_instance = nullptr;

	bool SceneManager::Init(uint32_t num_entries)
	{
		m_entries = num_entries;
		m_scenes.resize(m_entries);

		m_hashTable.Init(m_entries);
		m_hashTable.Fill(INVALID_ID);

		for (uint32_t i = 0; i < m_entries; i++)
		{
			Scene& scene = m_scenes[i];
			scene.m_id = INVALID_ID;
		}

		return true;
	}

	void SceneManager::Shutdown()
	{

		for (uint32_t i = 0; i < m_entries; i++)
		{
			Scene& scene = m_scenes[i];
			if (scene.m_id != INVALID_ID)
			{
				scene.OnDestroy();
				scene.m_id = INVALID_ID;
			}
		}

		m_scenes.clear();
	}

	Ref<Scene> SceneManager::CreateScene()
	{
		//TODO: Implement Hash Table Lookup
		Scene* result = nullptr;
		for (uint32_t i = 0; i < m_entries; i++)
		{
			Scene& scene = m_scenes[i];
			if (scene.m_id == INVALID_ID)
			{
				scene.OnCreate();
				result = &scene;
				break;
			}
		}

		return { result, BIND_RESOURCE_UNLOAD_FN(SceneManager::UnloadScene, this)};
	}
	Ref<Scene> SceneManager::CreateScene(const std::string& file_path)
	{
		TRC_ASSERT(false, "Implement Funtion {}", __FUNCTION__);
		return { nullptr, BIND_RESOURCE_UNLOAD_FN(SceneManager::UnloadScene, this) };
	}
	void SceneManager::UnloadScene(Scene* scene)
	{
		if (scene->m_refCount > 0)
		{
			TRC_WARN("{} scene is still in use", __FUNCTION__);//TODO: Replace __FUNCION__ with scene name
			return;
		}

		scene->m_id = INVALID_ID;
		scene->OnDestroy();

	}
	SceneManager* SceneManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new SceneManager();
		}
		return s_instance;
	}
}