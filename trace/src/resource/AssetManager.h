#pragma once

#include "Ref.h"
#include "scene/UUID.h"
#include "serialize/AssetsInfo.h"
#include "HashTable.h"
#include "core/io/Logging.h"

#include <vector>

namespace trace {

	template<typename T>
	class AssetManager
	{

	public:
		
		virtual bool Init(uint32_t max_units)
		{
			m_numUnits = max_units;
			m_hashtable.Init(max_units);
			m_hashtable.Fill(INVALID_ID);

			m_assets.resize(m_numUnits);

			return true;
		}

		virtual void Shutdown()
		{
			for (T& asset : m_assets)
			{
				if (asset.m_id == INVALID_ID)
					continue;
				UnLoad(&asset);
				TRC_TRACE("Asset was still in use, name : {}, RefCount : {}", asset.GetName(), asset.m_refCount);
			}
		}

		virtual Ref<T> Get(const std::string& name)
		{
			Ref<T> result;
			return result;
		}

		virtual void UnLoad(T* asset)
		{
			if (asset->m_refCount > 0)
			{
				TRC_WARN("{} asset is still in use", __FUNCTION__);
				return;
			}

			asset->m_id = INVALID_ID;
			m_hashtable.Set(asset->GetName(), INVALID_ID);
		}

		virtual void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}

		virtual Ref<T> Load_Runtime(UUID id)
		{
			return Ref<T>();
		}

	private:
	protected:
		std::vector<T> m_assets;
		HashTable<uint32_t> m_hashtable;
		uint32_t m_numUnits;
		std::unordered_map<UUID, AssetHeader> m_assetMap;

	protected:

		T* GetAsset(const std::string& name)
		{

			T* _asset = nullptr;
			uint32_t& _id = m_hashtable.Get_Ref(name);
			if (_id == INVALID_ID)
			{
				TRC_WARN("These scene has not been created , \"{}\"", name);
				return _asset;
			}
			_asset = &m_assets[_id];
			if (_asset->m_id == INVALID_ID)
			{
				TRC_WARN("These scene has been destroyed , \"{}\"", name);
				_id = INVALID_ID;
				return nullptr;
			}

			return _asset;
		}

		T* GetNextValidHandle(const std::string& name)
		{
			T* _asset = nullptr;

			for (uint32_t i = 0; i < m_numUnits; i++)
			{
				T& asset = m_assets[i];
				if (asset.m_id == INVALID_ID)
				{
					m_hashtable.Set(name, i);
					_asset = &asset;
					_asset->m_id = i;
					break;
				}
			}

			return _asset;
		}

		void InvalidateHandle(T* handle, const std::string& name)
		{
			m_hashtable.Set(name, INVALID_ID);

			handle->m_id = INVALID_ID;
		}

	};

}