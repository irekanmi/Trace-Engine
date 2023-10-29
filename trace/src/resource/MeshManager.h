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

		Ref<Mesh> GetMesh(const std::string& name);
		Ref<Mesh> LoadMesh(const std::string& name);
		Ref<Mesh> LoadMesh_(const std::string& path);
		Ref<Mesh> LoadMeshOnly_(const std::string& path);
		void Unload(Mesh* _mesh);
		Ref<Mesh> GetDefault(const std::string& name);
		std::vector<std::string> GetAllModels(const std::string& path);
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
		std::filesystem::path mesh_resource_path;

		void unloadDefaultModels(Model* model);
		bool LoadMesh_OBJ(std::filesystem::path& name);
		bool LoadMesh__OBJ(std::filesystem::path& name);
		bool LoadMeshOnly__OBJ(std::filesystem::path& name);
		std::vector<std::string> GetAllModels_OBJ(std::filesystem::path& path);

	protected:


	};

}
