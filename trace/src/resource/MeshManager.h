#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Mesh.h"
#include "HashTable.h"


namespace trace {

	class TRACE_API MeshManager
	{

	public:
		MeshManager();
		MeshManager(uint32_t max_entries);
		~MeshManager();

		bool Init(uint32_t max_entries);
		void ShutDown();

		Mesh* GetMesh(const std::string& name);
		bool LoadMesh(const std::string& name);
		void Unload(Mesh* mesh);
		Mesh* GetDefault(const std::string& name);
		bool LoadDefaults();

		static MeshManager* get_instance();
	private:
		static MeshManager* s_instance;

	private:
		HashTable<uint32_t> m_hashtable;
		std::vector<Mesh> m_meshes;
		uint32_t m_numEntries;
		Mesh DefaultCube;
		Model cube;

		void unloadDefaultModels(Model* model);
		bool LoadMesh_OBJ(const std::string& name);

	protected:


	};

}
