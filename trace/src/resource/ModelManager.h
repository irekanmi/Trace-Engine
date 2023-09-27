#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Model.h"
#include "HashTable.h"

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
		void UnLoadModel(Model* model);

		static ModelManager* get_instance();
	private:
		static ModelManager* s_instance;

	private:
		std::vector<Model> m_models;
		HashTable<uint32_t> m_hashtable;
		uint32_t m_numModelUnits;

	protected:

	};

}
