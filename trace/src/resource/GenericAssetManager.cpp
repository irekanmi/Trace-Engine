#include "pch.h"

#include "resource/GenericAssetManager.h"
#include "resource/DefaultAssetsManager.h"

namespace trace {

	void destroy_assets(std::unordered_map<UUID, Resource*>& assets)
	{
		for (auto [id, asset] : assets)
		{
			TRC_TRACE("Asset was still in use, name : {}, RefCount : {}", asset->GetName(), asset->m_refCount);
			asset->m_refCount = 0;
			asset->Destroy();
			//m_assetMap.erase(asset->GetUUID());

			delete asset;//TODO: Use custom memory allocator
		}

		assets.clear();
	}

	bool GenericAssetManager::Init(uint32_t max_units)
	{
		m_numUnits = max_units;

		return true;
	}
	void GenericAssetManager::Shutdown()
	{
		destroy_assets(m_assets[typeid(MaterialInstance)]);

		for (auto [id, asset] : m_assets)
		{
			destroy_assets(asset);
		}

		m_assets.clear();

	}
	

	void GenericAssetManager::BuildPipeline(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map)
	{
		for (auto& i : m_assets[typeid(GPipeline)])
		{
			if (GPipeline* pipeline = dynamic_cast<GPipeline*>(i.second))
			{
				Ref<GPipeline> asset = Get<GPipeline>(pipeline->GetUUID());
				DefaultAssetsManager::BuildPipeline(stream, map, asset);
			}
		}
	}


	GenericAssetManager* GenericAssetManager::get_instance()
	{
		static GenericAssetManager* s_instance = new GenericAssetManager;

		return s_instance;
	}
}