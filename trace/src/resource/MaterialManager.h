#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <stdint.h>
#include "HashTable.h"


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
		void Unload(MaterialInstance* material);
		bool LoadDefaults();

		static MaterialManager* get_instance();
	private:
		static MaterialManager* s_instance;

	private:
		std::vector<MaterialInstance> m_materials;
		Ref<MaterialInstance> default_material;
		uint32_t m_numEntries;
		HashTable<uint32_t> m_hashtable;

	protected:
	};

}
