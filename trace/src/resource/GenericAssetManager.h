#pragma once

#include "Ref.h"
#include "scene/UUID.h"
#include "serialize/AssetsInfo.h"
#include "HashTable.h"
#include "core/io/Logging.h"
#include "resource/Resource.h"
#include "serialize/FileStream.h"
#include "scene/UUID.h"
#include "core/Utils.h"
//TEMP -------
#include "render/GShader.h"
#include "render/Material.h"
//-----------

#include <unordered_map>

namespace trace {

	
	class GenericAssetManager
	{

	public:

		virtual bool Init(uint32_t max_units);


		virtual void Shutdown();
		

		template<typename T, typename ...Args>
		Ref<T> CreateAssetHandle(const std::string& name, Args ...args)
		{
			Ref<T> result;
			result = TryGet<T>(name);

			if (result)
			{
				return result;
			}

			result = Ref(GetNextValidHandle<T>((UUID)STR_ID(name)), BIND_RENDER_COMMAND_FN(GenericAssetManager::UnLoad));
			//TEMP: For debug only
			result->m_path = name;

			if (!result->Create(std::forward<Args>(args)...))
			{
				TRC_ERROR("Failed to create asset, Name: {}, Function: {}", name, __FUNCTION__);
				result.free();
				return Ref<T>();
			}

			return result;
		}

		template<typename T, typename ...Args>
		Ref<T> CreateAssetHandle_(const std::string& path, Args... args)
		{
			std::filesystem::path p(path);
			std::string name = p.filename().string();
			Ref<T> result;
			result = TryGet<T>(name);

			if (result)
			{
				return result;
			}

			result = Ref(GetNextValidHandle<T>((UUID)STR_ID(name)), BIND_RENDER_COMMAND_FN(GenericAssetManager::UnLoad));
			//TEMP: For debug only
			result->m_path = path;

			if (!result->Create(std::forward<Args>(args)...))
			{
				TRC_ERROR("Failed to create asset, Name: {}, Function: {}", name, __FUNCTION__);
				result.free();
				return Ref<T>();
			}

			Resource* asset = (Resource*)result.get();
			return result;
		}

		template<typename T>
		Ref<T> Get(const std::string& name)
		{
			return Get<T>((UUID)STR_ID(name));
		}
		
		template<typename T>
		Ref<T> Get(UUID asset_id)
		{
			Ref<T> result = TryGet<T>(asset_id);
			if (!result)
			{
				TRC_WARN("These asset has not been created or has been destroyed, Name: {}, Function: {}", STRING_FROM_ID(asset_id), __FUNCTION__);
			}
			return result;
		}
		
		template<typename T>
		Ref<T> TryGet(const std::string& name)
		{
			return TryGet<T>((UUID)STR_ID(name));
		}
		
		template<typename T>
		Ref<T> TryGet(UUID asset_id)
		{
			Ref<T> result;
			T* _asset = nullptr;
			/*uint32_t& _id = m_hashtable.Get_Ref(name);
			if (_id == INVALID_ID)
			{
				return result;
			}
			_asset = (T*)m_assets[_id];
			if (!_asset)
			{
				_id = INVALID_ID;
				return result;
			}
			else if (_asset->m_id == INVALID_ID)
			{
				_id = INVALID_ID;
				return result;
			}*/

			auto it = m_assets.find(asset_id);
			if (it == m_assets.end())
			{
				return result;
			}
			
			_asset = (T*)it->second;

			result = Ref{ _asset , BIND_RENDER_COMMAND_FN(GenericAssetManager::UnLoad) };
			return result;
		}

		template<typename T>
		void RenameAsset(Ref<T> asset, const std::string& new_name)
		{
			UUID new_id = STR_ID(new_name);
			UUID prev_id = asset->GetUUID();
			Resource* asset_data = m_assets[prev_id];
			m_assets.erase(prev_id);
			asset_data->m_assetID = new_id;
			m_assets[new_id] = asset_data;
		}

		virtual void UnLoad(Resource* asset);
		

		virtual void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}
		

		template<typename T>
		Ref<T> Load_Runtime(UUID id)
		{
			auto it = m_assetMap.find(id);
			if (it == m_assetMap.end())
			{
				return Ref<T>();
			}

			std::string bin_dir;
			FindDirectory(AppSettings::exe_path, "Data/generic.trbin", bin_dir);
			FileStream stream(bin_dir, FileMode::READ);

			stream.SetPosition(it->second.offset);

			return T::Deserialize(&stream);
		}

		template<typename T>
		void InvalidateHandle(Ref<T> handle)
		{
			if (!handle)
			{
				return;
			}
			const std::string& name = handle->GetName();
			m_hashtable.Set(name, INVALID_ID);

			m_assets[handle->m_id] = nullptr;
			handle->m_id = INVALID_ID;
			delete handle;//TODO: Use custom allocator
		}

		static GenericAssetManager* get_instance();
	private:
	protected:
		std::unordered_map<UUID, Resource*> m_assets;
		//HashTable<uint32_t> m_hashtable;
		uint32_t m_numUnits;
		std::unordered_map<UUID, AssetHeader> m_assetMap;


	protected:


		template<typename T>
		T* GetNextValidHandle(UUID asset_id)
		{
			T* _asset = nullptr;

			Resource* asset = nullptr;
			asset = new T; //TODO: Use custom memory allocator
			_asset = (T*)asset;
			m_assets[asset_id] = asset;
			_asset->m_assetID = asset_id;
			if constexpr (std::is_same<T, GShader>{} || std::is_same<T, MaterialInstance>{})
			{
				_asset->m_refCount++;
			}

			return _asset;
		}


	};

}
