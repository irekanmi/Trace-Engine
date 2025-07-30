#include "pch.h"

#include "resource/GenericAssetManager.h"

namespace trace {
	bool GenericAssetManager::Init(uint32_t max_units)
	{
		m_numUnits = max_units;
		//m_hashtable.Init(max_units);
		//m_hashtable.Fill(INVALID_ID);

		return true;
	}
	void GenericAssetManager::Shutdown()
	{

		//for (int32_t i = (m_numUnits - 1); i >= 0 ; i--)
		//{
		//	Resource* asset = m_assets[i];
		//	if (!asset)
		//	{
		//		continue;
		//	}
		//	if (asset->m_id == INVALID_ID)
		//	{
		//		continue;
		//	}
		//	TRC_TRACE("Asset was still in use, name : {}, RefCount : {}", asset->GetName(), asset->m_refCount);
		//	asset->m_refCount = 0;
		//	asset->Destroy();

		//	delete asset;//TODO: Use custom memory allocator
		//}

		for (auto [id, asset] : m_assets)
		{
			TRC_TRACE("Asset was still in use, name : {}, RefCount : {}", asset->GetName(), asset->m_refCount);
			asset->m_refCount = 0;
			asset->Destroy();

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


		m_assets.erase(asset->GetUUID());
		TRC_TRACE("{} is destroyed", asset->GetName());
		asset->Destroy();


		delete asset;//TODO: Use custom memory allocator
	}


	GenericAssetManager* GenericAssetManager::get_instance()
	{
		static GenericAssetManager* s_instance = new GenericAssetManager;

		return s_instance;
	}
}