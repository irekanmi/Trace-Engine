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

		bool CreateMaterial(const std::string& name, Material material, Ref<GPipeline> pipeline);
		MaterialInstance* GetMaterial(const std::string& name);
		void Unload(MaterialInstance* material);

		static MaterialManager* get_instance();
	private:
		static MaterialManager* s_instance;

	private:
		std::vector<MaterialInstance> m_materials;
		uint32_t m_numEntries;
		HashTable<uint32_t> m_hashtable;

	protected:
	};

}