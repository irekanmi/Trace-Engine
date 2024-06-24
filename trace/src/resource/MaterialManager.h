#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "HashTable.h"
#include "serialize/AssetsInfo.h"
#include "scene/UUID.h"
#include "render/Material.h"


#include <stdint.h>


namespace trace {

	class TRACE_API MaterialManager
	{
	public:
		MaterialManager();
		MaterialManager(uint32_t max_entries);
		~MaterialManager();


		bool Init(uint32_t max_entries);
		void ShutDown();

		Ref<MaterialInstance> CreateMaterial(const std::string& name, Ref<GPipeline> pipeline);
		Ref<MaterialInstance> GetMaterial(const std::string& name);
		bool RecreateMaterial(Ref<MaterialInstance> mat, Ref<GPipeline> pipeline);
		void Unload(MaterialInstance* material);
		bool LoadDefaults();
		void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}
		Ref<MaterialInstance> LoadMaterial_Runtime(UUID id);

		static MaterialManager* get_instance();
	private:
		static MaterialManager* s_instance;

	private:
		std::vector<MaterialInstance> m_materials;
		Ref<MaterialInstance> default_material;
		uint32_t m_numEntries;
		HashTable<uint32_t> m_hashtable;
		std::unordered_map<UUID, AssetHeader> m_assetMap;

	protected:
	};

}
