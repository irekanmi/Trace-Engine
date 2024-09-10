#pragma once

#include "Ref.h"
#include "scene/UUID.h"
#include "serialize/AssetsInfo.h"
#include "HashTable.h"
#include "core/io/Logging.h"
#include "resource/Resource.h"

#include <vector>

namespace trace {

	
	class GenericAssetManager
	{

	public:

		virtual bool Init(uint32_t max_units);


		virtual void Shutdown();
		

		template<typename T>
		Ref<T> CreateAssetHandle(const std::string& name)
		{
			Ref<T> result;
			result = Get<T>(name);

			if (result)
			{
				return result;
			}

			result = Ref(GetNextValidHandle<T>(name), BIND_RENDER_COMMAND_FN(GenericAssetManager::UnLoad));
			return result;
		}

		template<typename T>
		Ref<T> Get(const std::string& name)
		{
			Ref<T> result = Ref(GetAsset<T>(name), BIND_RENDER_COMMAND_FN(GenericAssetManager::UnLoad));
			return result;
		}

		virtual void UnLoad(Resource* asset);
		

		virtual void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}

		template<typename T>
		Ref<T> Load_Runtime(UUID id)
		{
			return Ref<T>();
		}

	private:
	protected:
		std::vector<Resource*> m_assets;
		HashTable<uint32_t> m_hashtable;
		uint32_t m_numUnits;
		std::unordered_map<UUID, AssetHeader> m_assetMap;

	protected:

		template<typename T>
		T* GetAsset(const std::string& name)
		{

			T* _asset = nullptr;
			uint32_t& _id = m_hashtable.Get_Ref(name);
			if (_id == INVALID_ID)
			{
				TRC_WARN("These scene has not been created , \"{}\"", name);
				return _asset;
			}
			_asset = m_assets[_id];
			if (_asset->m_id == INVALID_ID)
			{
				TRC_WARN("These scene has been destroyed , \"{}\"", name);
				_id = INVALID_ID;
				return nullptr;
			}

			return _asset;
		}

		template<typename T>
		T* GetNextValidHandle(const std::string& name)
		{
			T* _asset = nullptr;

			for (uint32_t i = 0; i < m_numUnits; i++)
			{
				Resource* asset = m_assets[i];
				if (!asset)
				{
					asset = new T; //TODO: Use custom memory allocator
					m_assets[i] = asset;
					m_hashtable.Set(name, i);
					_asset = asset;
					_asset->m_id = i;
					break;
				}
			}

			return _asset;
		}


	};

}
