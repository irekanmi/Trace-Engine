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
		asset->SetHandle(handle);

		result = { asset, BIND_RESOURCE_UNLOAD_FN(PrefabManager::UnLoad, this) };

		return result;
	}

	Ref<Prefab> PrefabManager::Get(const std::string& name)
	{
		Ref<Prefab> result;
		Prefab* asset = GetAsset(name);

		result = { asset, BIND_RESOURCE_UNLOAD_FN(PrefabManager::UnLoad, this) };
		return result;
	}

	void PrefabManager::UnLoad(Prefab* asset)
	{
		AssetManager::UnLoad(asset);
		m_prefabScene->DestroyEntity(asset->GetHandle());
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