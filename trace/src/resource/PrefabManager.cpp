#include "pch.h"

#include "PrefabManager.h"
#include "scene/Scene.h"

namespace trace {



	bool PrefabManager::Init(uint32_t num_units)
	{
		bool result = AssetManager::Init(num_units);

		m_prefabScene = new Scene;// TODO: Use custom allocator
		m_prefabScene->Create();
		m_prefabScene->SetName("PreFab Edit");
		return result;
	}

	void PrefabManager::Shutdown()
	{
		AssetManager::Shutdown();
		m_prefabScene->Destroy();
		delete m_prefabScene; //TODO: Use custom allocator
	}

	Ref<Prefab> PrefabManager::Create(const std::string& name)
	{
		Ref<Prefab> result;
		result = Get(name);
		if (result) return result;

		Prefab* asset = GetNextValidHandle(name);
		Entity handle = m_prefabScene->CreateEntity();
		asset->SetHandle(handle.GetID());
		asset->m_path = name;

		result = { asset, BIND_RESOURCE_UNLOAD_FN(PrefabManager::UnLoad, this) };

		return result;
	}

	Ref<Prefab> PrefabManager::Create(const std::string& name, Entity handle)
	{
		Ref<Prefab> result;
		result = Get(name);
		if (result) return result;

		Prefab* asset = GetNextValidHandle(name);
		Entity p = m_prefabScene->DuplicateEntity(handle);
		asset->SetHandle(p.GetID());
		asset->m_path = name;

		result = { asset, BIND_RESOURCE_UNLOAD_FN(PrefabManager::UnLoad, this) };

		return result;
	}

	Ref<Prefab> PrefabManager::Get(const std::string& name)
	{
		Ref<Prefab> result;
		Prefab* asset = GetAsset(name);

		if(asset) result = { asset, BIND_RESOURCE_UNLOAD_FN(PrefabManager::UnLoad, this) };
		return result;
	}

	void PrefabManager::UnLoad(Resource* res)
	{
		Prefab* asset = (Prefab*)res;
		AssetManager::UnLoad(asset);
		m_prefabScene->DestroyEntity(m_prefabScene->GetEntity(asset->GetHandle()));
	}

	Ref<Prefab> PrefabManager::Load_Runtime(UUID id)
	{
		return Ref<Prefab>();
	}

	PrefabManager* PrefabManager::get_instance()
	{
		static PrefabManager* s_instance = new PrefabManager;
		return s_instance;
	}

}