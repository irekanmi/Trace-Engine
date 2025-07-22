#include "pch.h"

#include "resource/GenericAssetManager.h"

namespace trace {
	bool GenericAssetManager::Init(uint32_t max_units)
	{
		m_numUnits = max_units;
		m_hashtable.Init(max_units);
		m_hashtable.Fill(INVALID_ID);

		m_assets.resize(m_numUnits);

		return true;
	}
	void GenericAssetManager::Shutdown()
	{

		for (int32_t i = (m_numUnits - 1); i >= 0 ; i--)
		{
			Resource* asset = m_assets[i];
			if (!asset)
			{
				continue;
			}
			if (asset->m_id == INVALID_ID)
			{
				continue;
			}
			TRC_TRACE("Asset was still in use, name : {}, RefCount : {}", asset->GetName(), asset->m_refCount);
			asset->m_refCount = 0;
			asset->Destroy();

			delete asset;//TODO: Use custom memory allocator
		}
	}
	void GenericAssetManager::UnLoad(Resource* asset)
	{
		if (asset->m_refCount > 0)
		{
			TRC_WARN("{} asset is still in use", __FUNCTION__);
			return;
		}

		TRC_ASSERT(asset->m_id < m_assets.size(), "Assets id is greater than avaliable size");

		m_assets[asset->m_id] = nullptr;
		asset->m_id = INVALID_ID;
		TRC_TRACE("{} is destroyed", asset->GetName());
		m_hashtable.Set(asset->GetName(), INVALID_ID);
		asset->Destroy();


		delete asset;//TODO: Use custom memory allocator
	}


	GenericAssetManager* GenericAssetManager::get_instance()
	{
		static GenericAssetManager* s_instance = new GenericAssetManager;

		return s_instance;
	}
}