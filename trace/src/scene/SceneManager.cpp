#include "pch.h"

#include "SceneManager.h"
#include "core/io/Logging.h"
#include "core/Enums.h"
#include "core/Utils.h"
#include "core/Coretypes.h"
#include "serialize/FileStream.h"
#include "serialize/SceneSerializer.h"

namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);


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
				scene.Destroy();
				scene.m_id = INVALID_ID;
			}
		}

		m_scenes.clear();
	}

	Ref<Scene> SceneManager::CreateScene(const std::string& name)
	{

		Ref<Scene> result;
		Scene* _scene = nullptr;
		result = GetScene(name);
		if (result)
		{
			return result;
		}

		for (uint32_t i = 0; i < m_entries; i++)
		{
			Scene& scene = m_scenes[i];
			if (scene.m_id == INVALID_ID)
			{
				scene.Create();
				m_hashTable.Set(name, i);
				_scene = &scene;
				_scene->SetName(name);
				_scene->m_id = i;
				break;
			}
		}
		
		result = { _scene, BIND_RESOURCE_UNLOAD_FN(SceneManager::UnloadScene, this) };
		return result;
	}
	Ref<Scene> SceneManager::GetScene(const std::string& name)
	{
		Ref<Scene> result;
		Scene* _scene = nullptr;
		uint32_t& _id = m_hashTable.Get_Ref(name);
		if (_id == INVALID_ID)
		{
			TRC_WARN("These scene has not been created , \"{}\"", name);
			return result;
		}
		_scene = &m_scenes[_id];
		if (_scene->m_id == INVALID_ID)
		{
			TRC_WARN("These scene has been destroyed , \"{}\"", name);
			_id = INVALID_ID;
			return result;
		}
		result = { _scene, BIND_RESOURCE_UNLOAD_FN(SceneManager::UnloadScene, this) };
		return result;
	}
	Ref<Scene> SceneManager::CreateScene_(const std::string& file_path)
	{
		TRC_ASSERT(false, "Implement Funtion {}", __FUNCTION__);
		return { nullptr, BIND_RESOURCE_UNLOAD_FN(SceneManager::UnloadScene, this) };
	}
	void SceneManager::UnloadScene(Resource* res)
	{
		Scene* scene = (Scene*)res;

		if (scene->m_refCount > 0)
		{
			TRC_WARN("{} scene is still in use", __FUNCTION__);//TODO: Replace __FUNCION__ with scene name
			return;
		}

		scene->m_id = INVALID_ID;
		scene->Destroy();

	}
	Ref<Scene> SceneManager::LoadScene_Runtime(UUID id)
	{
		Ref<Scene> result;
		Scene* _scene = nullptr;

		auto it = m_assetMap.find(id);
		if (it == m_assetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		std::string name = GetNameFromUUID(id);

		result = GetScene(name);
		if (result)
		{
			return result;
		}

		for (uint32_t i = 0; i < m_entries; i++)
		{
			Scene& scene = m_scenes[i];
			if (scene.m_id == INVALID_ID)
			{
				scene.Create();
				m_hashTable.Set(name, i);
				_scene = &scene;
				_scene->SetName(name);
				_scene->m_id = i;
				break;
			}
		}

		result = { _scene, BIND_RESOURCE_UNLOAD_FN(SceneManager::UnloadScene, this) };

		std::string bin_dir;
		FindDirectory(AppSettings::exe_path, "Data/trscn.trbin", bin_dir);
		FileStream stream(bin_dir, FileMode::READ);
		SceneSerializer::Deserialize(result, stream, it->second);

		return result;
	}
	SceneManager* SceneManager::get_instance()
	{
		static SceneManager* s_instance = new SceneManager;
		return s_instance;
	}
}