#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Model.h"
#include "HashTable.h"
#include "serialize/AssetsInfo.h"
#include "scene/UUID.h"

namespace trace {

	class TRACE_API ModelManager
	{

	public:
		ModelManager();
		~ModelManager();

		bool Init(uint32_t max_units);
		void ShutDown();

		Ref<Model> GetModel(const std::string& name);
		Ref<Model> LoadModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& name);
		Ref<Model> LoadModel_(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::string& path);
		void UnLoadModel(Model* model);
		void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}
		Ref<Model> LoadModel_Runtime(UUID id);

		static ModelManager* get_instance();
	private:

	private:
		std::vector<Model> m_models;
		HashTable<uint32_t> m_hashtable;
		uint32_t m_numModelUnits;
		std::unordered_map<UUID, AssetHeader> m_assetMap;

	protected:

	};

}
