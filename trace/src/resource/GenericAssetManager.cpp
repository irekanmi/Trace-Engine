#include "pch.h"

#include "resource/GenericAssetManager.h"
#include "resource/DefaultAssetsManager.h"

namespace trace {
	bool GenericAssetManager::Init(uint32_t max_units)
	{
		m_numUnits = max_units;

		return true;
	}
	void GenericAssetManager::Shutdown()
	{

		for (auto [id, asset] : m_assets)
		{
			TRC_TRACE("Asset was still in use, name : {}, RefCount : {}", asset->GetName(), asset->m_refCount);
			asset->m_refCount = 0;
			asset->Destroy();
			m_assetMap.erase(asset->GetUUID());

			delete asset;//TODO: Use custom memory allocator
		}

		m_assets.clear();

	}
	void GenericAssetManager::UnLoad(Resource* asset)
	{
		if (asset->m_refCount > 0)
		{
			TRC_WARN("{} asset is still in use", __FUNCTION__);
			return;
		}
		
		auto it = m_assets.find(asset->GetUUID());
		if (it == m_assets.end())
		{
			TRC_WARN("These not suppose to happen, Asset ID: {}, Name: {}, Function: {}", asset->GetUUID(), STRING_FROM_ID(asset->GetUUID()), __FUNCTION__);
			return;
		}


		asset->Destroy();
		TRC_TRACE("{} is destroyed", asset->GetName());
		m_assets.erase(asset->GetUUID());
		asset->m_assetID = 0;


		delete asset;//TODO: Use custom memory allocator
	}

	void GenericAssetManager::BuildPipeline(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map)
	{
		for (auto& i : m_assets)
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