#include "pch.h"

#include "PrefabManager.h"
#include "scene/Scene.h"
#include "core/Utils.h"
#include "core/Coretypes.h"
#include "serialize/FileStream.h"
#include "serialize/MemoryStream.h"
#include "serialize/SceneSerializer.h"

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
		Ref<Prefab> result;
		auto it = m_assetsMap.find(id);
		if (it == m_assetsMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}


		std::string bin_dir;
		FindDirectory(AppSettings::exe_path, "Data/trprf.trbin", bin_dir);
		FileStream stream(bin_dir, FileMode::READ);

		stream.SetPosition(it->second.offset);

		result = SceneSerializer::DeserializePrefab(&stream);
		return result;
	}

	void PrefabManager::SetAssetMap(std::unordered_map<UUID, AssetHeader>& map)
	{
		m_assetsMap = map;
	}

	PrefabManager* PrefabManager::get_instance()
	{
		static PrefabManager* s_instance = new PrefabManager;
		return s_instance;
	}

}