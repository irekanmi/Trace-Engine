#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Mesh.h"
#include "HashTable.h"
#include <filesystem>


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
		void Unload(Mesh* _mesh);
		Ref<Mesh> GetDefault(const std::string& name);
		bool LoadDefaults();

		static MeshManager* get_instance();
	private:
		static MeshManager* s_instance;

	private:
		HashTable<uint32_t> m_hashtable;
		std::vector<Mesh> m_meshes;
		uint32_t m_numEntries;
		Ref<Mesh> DefaultCube;
		Ref<Mesh> DefaultSphere;
		Model cube;
		Model sphere;
		std::filesystem::path mesh_resource_path;

		void unloadDefaultModels(Model* model);
		bool LoadMesh_OBJ(const std::string& name);
		bool LoadMesh__OBJ(const std::string& name);

	protected:


	};

}
