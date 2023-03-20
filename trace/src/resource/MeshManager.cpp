#include "pch.h"

#include "MeshManager.h"
#include "OBJ_Loader.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "PipelineManager.h"
#include "MaterialManager.h"


namespace trace {
	MeshManager* MeshManager::s_instance = nullptr;


	MeshManager::MeshManager()
	{
	}

	MeshManager::MeshManager(uint32_t max_entries)
	{
		Init(max_entries);
	}

	MeshManager::~MeshManager()
	{
	}

	bool MeshManager::Init(uint32_t max_entries)
	{
		m_numEntries = max_entries;
		m_hashtable.Init(max_entries);
		m_hashtable.Fill(INVALID_ID);

		m_meshes.resize(m_numEntries);



		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_meshes[i].m_id = INVALID_ID;
		}

		return true;
	}

	void MeshManager::ShutDown()
	{
		if (!m_meshes.empty())
		{
			DefaultCube.~Mesh();

			m_meshes.clear();
		}
	}

	Mesh* MeshManager::GetMesh(const std::string& name)
	{
		uint32_t _id = m_hashtable.Get(name);
		if (_id == INVALID_ID)
		{
			TRC_WARN("Please ensure mesh loaded before getting; trying to load");
			if (LoadMesh(name))
			{
				return &m_meshes[m_hashtable.Get(name)];
			}
		}

		return &m_meshes[m_hashtable.Get(name)];
	}

	bool MeshManager::LoadMesh(const std::string& name)
	{
		bool result = false;
		if (m_hashtable.Get(name) != INVALID_ID)
		{
			result = true;
			TRC_WARN("mesh has already loaded %s", name.c_str());
			return result;
		}

		result = LoadMesh_OBJ(name);

		return result;
	}

	void MeshManager::Unload(Mesh* mesh)
	{

		if (mesh->m_refCount > 0)
		{
			TRC_WARN("Current mesh is still in use can't unload");
			return;
		}

		mesh->m_id = INVALID_ID;
		mesh->~Mesh();

	}

	Mesh* MeshManager::GetDefault(const std::string& name)
	{

		if (m_hashtable.Hash(name) == m_hashtable.Hash("Cube"))
		{
			return &DefaultCube;
		}

		return nullptr;
	}

	bool MeshManager::LoadDefaults()
	{

		std::vector<Vertex> verts;
		std::vector<uint32_t> _ind;

		generateDefaultCube(verts, _ind);
		generateVertexTangent(verts, _ind);
		cube.Init(verts, _ind);

		Ref<Model> cube_ref(&cube, BIND_RESOURCE_UNLOAD_FN(MeshManager::unloadDefaultModels, this));

		DefaultCube.GetModels().push_back(cube_ref);

		return true;
	}

	MeshManager* MeshManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new MeshManager();
		}
		return s_instance;
	}

	void MeshManager::unloadDefaultModels(Model* model)
	{
		if (model->m_refCount > 0)
		{
			TRC_WARN("Model is still in use can't unload");
			return;
		}

		model->m_id = INVALID_ID;
		model->~Model();

	}

	bool MeshManager::LoadMesh_OBJ(const std::string& name)
	{
		bool result = false;

		objl::Loader loader;

		result = loader.LoadFile(("../assets/meshes/" + name));
		if (result)
		{
			ModelManager* model_manager = ModelManager::get_instance();
			TextureManager* texture_manager = TextureManager::get_instance();
			PipelineManager* pipeline_manager = PipelineManager::get_instance();
			MaterialManager* material_manager = MaterialManager::get_instance();
			Mesh* _mesh = nullptr;
			uint32_t _id = INVALID_ID;
			for (uint32_t k = 0; k < m_numEntries; k++)
			{
				if (m_meshes[k].m_id == INVALID_ID)
				{
					m_meshes[k].m_id = k;
					_id = k;
					m_hashtable.Set(name, k);
					_mesh = &m_meshes[k];
					break;
				}
			}
			for (uint32_t i = 0; i < loader.LoadedMeshes.size(); i++)
			{
				objl::Mesh mesh = loader.LoadedMeshes[i];

				std::vector<Vertex> vert;
				std::vector<uint32_t> _ind;

				vert.reserve(mesh.Vertices.size());
				for (uint32_t j = 0; j < mesh.Vertices.size(); j++)
				{
					Vertex current_vertex;
					current_vertex.pos.x = mesh.Vertices[j].Position.X;
					current_vertex.pos.y = mesh.Vertices[j].Position.Y;
					current_vertex.pos.z = mesh.Vertices[j].Position.Z;

					current_vertex.normal.x = mesh.Vertices[j].Normal.X;
					current_vertex.normal.y = mesh.Vertices[j].Normal.Y;
					current_vertex.normal.z = mesh.Vertices[j].Normal.Z;

					current_vertex.texCoord.x = mesh.Vertices[j].TextureCoordinate.X;
					current_vertex.texCoord.y = mesh.Vertices[j].TextureCoordinate.Y;

					vert.push_back(current_vertex);
				}

				_ind.resize(mesh.Indices.size());
				for (uint32_t j = 0; j < mesh.Indices.size(); j += 3)
				{
					_ind[j + 0] = mesh.Indices[j + 0];
					_ind[j + 1] = mesh.Indices[j + 1];
					_ind[j + 2] = mesh.Indices[j + 2];
				}

				generateVertexTangent(vert, _ind);

				Material mat;
				mat.m_albedoMap = texture_manager->GetDefault("albedo_map");
				mat.m_normalMap = texture_manager->GetDefault("normal_map");
				mat.m_specularMap = texture_manager->GetDefault("specular_map");
				if (!mesh.MeshMaterial.map_Kd.empty())
				{
					if (texture_manager->LoadTexture(mesh.MeshMaterial.map_Kd))
					{
						mat.m_albedoMap = { texture_manager->GetTexture(mesh.MeshMaterial.map_Kd), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, texture_manager) };
					}
					else
					{
						TRC_WARN("Failed to load texture %s", mesh.MeshMaterial.map_Kd.c_str());
					}
				}
				if (!mesh.MeshMaterial.map_Ks.empty())
				{
					if (texture_manager->LoadTexture(mesh.MeshMaterial.map_Ks))
					{
						mat.m_specularMap = { texture_manager->GetTexture(mesh.MeshMaterial.map_Ks), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, texture_manager) };
					}
					else
					{
						TRC_WARN("Failed to load texture %s", mesh.MeshMaterial.map_Ks.c_str());
					}
				}
				if (!mesh.MeshMaterial.map_bump.empty())
				{
					TextureDesc texture_desc;
					texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
					texture_desc.m_format = Format::R8G8B8A8_UNORM;
					texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
					texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
					texture_desc.m_usage = UsageFlag::DEFAULT;
					if (texture_manager->LoadTexture(mesh.MeshMaterial.map_bump, texture_desc))
					{
						mat.m_normalMap = { texture_manager->GetTexture(mesh.MeshMaterial.map_bump), BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadTexture, texture_manager) };
					}
					else
					{
						TRC_WARN("Failed to load texture %s", mesh.MeshMaterial.map_bump.c_str());
					}
				}
				mat.m_diffuseColor = {
					mesh.MeshMaterial.Kd.X,
					mesh.MeshMaterial.Kd.Y,
					mesh.MeshMaterial.Kd.Z,
					1.0f
				};

				mat.m_shininess = mesh.MeshMaterial.Ns;

				Model* _model = nullptr;
				if (model_manager->LoadModel(
					vert,
					_ind,
					mesh.MeshName
				))
				{
					_model = model_manager->GetModel(mesh.MeshName);
					Ref<GPipeline> sp = pipeline_manager->GetDefault("standard");
					material_manager->CreateMaterial(
						mesh.MeshMaterial.name,
						mat,
						sp
					);

					Ref<MaterialInstance> _mi = {material_manager->GetMaterial(mesh.MeshMaterial.name), BIND_RESOURCE_UNLOAD_FN(MaterialManager::Unload, material_manager)};
					_model->m_matInstance = _mi;
				}

				_mesh->GetModels().push_back({ _model, BIND_RESOURCE_UNLOAD_FN(ModelManager::UnLoadModel, model_manager) });

			}

		}

		return result;
	}

}