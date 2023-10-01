#include "pch.h"

#include "MeshManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "PipelineManager.h"
#include "MaterialManager.h"
#include "OBJ_Loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace std {
	template<> struct hash<trace::Vertex> {
		size_t operator()(trace::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

namespace trace {
	MeshManager* MeshManager::s_instance = nullptr;


	MeshManager::MeshManager()
	{
	}

	MeshManager::MeshManager(uint32_t max_entries)
	{
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

		// Trying to determine directory where meshes are located..............
		std::filesystem::path current_search_path("./assets");

		if (std::filesystem::exists(current_search_path))
		{
			current_search_path /= "meshes";
			if (std::filesystem::exists(current_search_path))
			{
				mesh_resource_path = current_search_path;
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "meshes";
				if (std::filesystem::exists(current_search_path))
				{
					mesh_resource_path = current_search_path;
				}
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "meshes";
				if (std::filesystem::exists(current_search_path))
				{
					mesh_resource_path = current_search_path;
				}
			}
		}
		else
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "meshes";
				if (std::filesystem::exists(current_search_path))
				{
					mesh_resource_path = current_search_path;
				}
			}
		}
		// .............................

		return true;
	}

	void MeshManager::ShutDown()
	{
		DefaultCube.~Ref();
		DefaultSphere.~Ref();

		if (m_meshes.empty())
			return;

		for (Mesh& _mesh : m_meshes)
		{
			if (_mesh.m_id != INVALID_ID)
			{
				Unload(&_mesh);
				TRC_TRACE("Mesh not released , id : {}", _mesh.m_id);
			}
		}
		m_meshes.clear();
	}

	Ref<Mesh> MeshManager::GetMesh(const std::string& name)
	{
		Ref<Mesh> result;
		Mesh* _mesh = nullptr;
		uint32_t& _id = m_hashtable.Get_Ref(name);
		if (_id == INVALID_ID)
		{
			TRC_WARN("Please ensure mesh loaded before getting {}", name);
			return result;
			
		}
		_mesh = &m_meshes[_id];
		if (_mesh->m_id == INVALID_ID)
		{
			TRC_WARN("{} mesh has been destroyed", name);
			_id = INVALID_ID;
			return result;
		}
		result = { _mesh, BIND_RENDER_COMMAND_FN(MeshManager::Unload) };
		return result;
	}

	Ref<Mesh> MeshManager::LoadMesh(const std::string& name)
	{
		return LoadMesh_((mesh_resource_path / name).string());
	}

	Ref<Mesh> MeshManager::LoadMesh_(const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		Ref<Mesh> result;
		Mesh* _mesh = nullptr;
		result = GetMesh(name);
		if (result)
		{
			TRC_WARN("mesh has already loaded {}", name);
			return result;
		}
		if (p.extension() == ".obj")
		{
			if (LoadMesh__OBJ(p))
			{
				return GetMesh(name);
			}

		}


		return result;
	}

	void MeshManager::Unload(Mesh* _mesh)
	{

		if (_mesh->m_refCount > 0)
		{
			TRC_WARN("Current mesh is still in use can't unload");
			return;
		}



		_mesh->m_id = INVALID_ID;
		_mesh->~Mesh();

	}

	Ref<Mesh> MeshManager::GetDefault(const std::string& name)
	{

		if (m_hashtable.Hash(name) == m_hashtable.Hash("Cube"))
		{
			return DefaultCube;
		}
		else if (m_hashtable.Hash(name) == m_hashtable.Hash("Sphere"))
		{
			return DefaultSphere;
		}

		return { nullptr, BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, this)};
	}

	bool MeshManager::LoadDefaults()
	{

		std::vector<Vertex> verts;
		std::vector<uint32_t> _ind;

		generateDefaultCube(verts, _ind);
		generateVertexTangent(verts, _ind);
		Ref<Model> cube_ref = ModelManager::get_instance()->LoadModel(verts, _ind, "Cube");
		cube_ref->m_matInstance = MaterialManager::get_instance()->GetMaterial("default");

		Mesh* _mesh = nullptr;
		uint32_t _id = INVALID_ID;
		for (uint32_t k = 0; k < m_numEntries; k++)
		{
			if (m_meshes[k].m_id == INVALID_ID)
			{
				m_meshes[k].m_id = k;
				_id = k;
				m_hashtable.Set("Cube", k);
				_mesh = &m_meshes[k];
				break;
			}
		}


		DefaultCube = { _mesh, BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, this) };
		_mesh = nullptr;
		DefaultCube->GetModels().push_back(cube_ref);
		DefaultCube->m_path = "Cube";

		verts.clear();
		_ind.clear();

		generateSphere(verts, _ind, 5.0f, 50, 50);
		generateVertexTangent(verts, _ind);
		Ref<Model> sphere_ref = ModelManager::get_instance()->LoadModel(verts, _ind, "Sphere");
		sphere_ref->m_matInstance = MaterialManager::get_instance()->GetMaterial("default");


		for (uint32_t k = 0; k < m_numEntries; k++)
		{
			if (m_meshes[k].m_id == INVALID_ID)
			{
				m_meshes[k].m_id = k;
				_id = k;
				m_hashtable.Set("Cube", k);
				_mesh = &m_meshes[k];
				break;
			}
		}
		DefaultSphere = { _mesh, BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, this) };
		DefaultSphere->GetModels().push_back(sphere_ref);
		DefaultSphere->m_path = "Sphere";

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

		//TODO: 
		model->m_id = INVALID_ID;
		model->~Model();

	}

	bool MeshManager::LoadMesh_OBJ(std::filesystem::path& path)
	{
		bool result = false;

		objl::Loader loader;
		std::string name = path.filename().string();
		result = loader.LoadFile(path.string());
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
					_mesh->m_path = path;
					break;
				}
			}
			for (uint32_t i = 0; i < loader.LoadedMeshes.size(); i++)
			{
				objl::Mesh obj_mesh = loader.LoadedMeshes[i];

				std::vector<Vertex> vert;
				std::vector<uint32_t> _ind;

				vert.reserve(obj_mesh.Vertices.size());
				for (uint32_t j = 0; j < obj_mesh.Vertices.size(); j++)
				{
					Vertex current_vertex;
					current_vertex.pos.x = obj_mesh.Vertices[j].Position.X;
					current_vertex.pos.y = obj_mesh.Vertices[j].Position.Y;
					current_vertex.pos.z = obj_mesh.Vertices[j].Position.Z;

					current_vertex.normal.x = obj_mesh.Vertices[j].Normal.X;
					current_vertex.normal.y = obj_mesh.Vertices[j].Normal.Y;
					current_vertex.normal.z = obj_mesh.Vertices[j].Normal.Z;

					current_vertex.texCoord.x = obj_mesh.Vertices[j].TextureCoordinate.X;
					current_vertex.texCoord.y = obj_mesh.Vertices[j].TextureCoordinate.Y;

					vert.push_back(current_vertex);
				}

				_ind.resize(obj_mesh.Indices.size());
				for (uint32_t j = 0; j < obj_mesh.Indices.size(); j += 3)
				{
					_ind[j + 0] = obj_mesh.Indices[j + 0];
					_ind[j + 1] = obj_mesh.Indices[j + 1];
					_ind[j + 2] = obj_mesh.Indices[j + 2];
				}

				generateVertexTangent(vert, _ind);

				Material mat;
				mat.m_albedoMap = texture_manager->GetDefault("albedo_map");
				mat.m_normalMap = texture_manager->GetDefault("normal_map");
				mat.m_specularMap = texture_manager->GetDefault("specular_map");
				if (!obj_mesh.MeshMaterial.map_Kd.empty())
				{

					if (Ref<GTexture> albe = texture_manager->LoadTexture(obj_mesh.MeshMaterial.map_Kd))
					{
						mat.m_albedoMap = albe;
					}
					else
					{
						TRC_WARN("Failed to load texture {}", obj_mesh.MeshMaterial.map_Kd);
					}
				}
				if (!obj_mesh.MeshMaterial.map_Ks.empty())
				{
					if (Ref<GTexture> spec = texture_manager->LoadTexture(obj_mesh.MeshMaterial.map_Ks))
					{
						mat.m_specularMap = spec;
					}
					else
					{
						TRC_WARN("Failed to load texture {}", obj_mesh.MeshMaterial.map_Ks);
					}
				}
				if (!obj_mesh.MeshMaterial.map_bump.empty())
				{
					TextureDesc texture_desc;
					texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
					texture_desc.m_format = Format::R8G8B8A8_UNORM;
					texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
					texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
					texture_desc.m_usage = UsageFlag::DEFAULT;
					if (Ref<GTexture> nrm = texture_manager->LoadTexture(obj_mesh.MeshMaterial.map_bump, texture_desc))
					{
						mat.m_normalMap = nrm;
					}
					else
					{
						TRC_WARN("Failed to load texture {}", obj_mesh.MeshMaterial.map_bump);
					}
				}
				mat.m_diffuseColor = {
					obj_mesh.MeshMaterial.Kd.X,
					obj_mesh.MeshMaterial.Kd.Y,
					obj_mesh.MeshMaterial.Kd.Z,
					1.0f
				};

				mat.m_shininess = obj_mesh.MeshMaterial.Ns;

				Ref<Model> _model = model_manager->LoadModel_(vert,_ind,( path / obj_mesh.MeshName).string());
				if (_model)
				{
					Ref<GPipeline> sp = pipeline_manager->GetPipeline("gbuffer_pipeline");
					Ref<MaterialInstance> _mi = material_manager->CreateMaterial(obj_mesh.MeshMaterial.name,mat,sp);
					_model->m_matInstance = _mi;
				}

				_mesh->GetModels().push_back(_model);

			}

		}

		return result;
	}

	bool MeshManager::LoadMesh__OBJ(std::filesystem::path& path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string parent_path = (path.parent_path() / "").generic_string();

		bool result = true;
		std::string name = path.filename().string();
		result = tinyobj::LoadObj(
			&attrib,
			&shapes,
			&materials,
			&err,
			path.string().c_str(),
			parent_path.c_str()
		);

		if (!result)
		{
			TRC_ERROR("Failed to load mesh {}", name);
			TRC_ERROR(err);
			return result;
		}

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
				_mesh->m_path = path;
				break;
			}
		}
		std::unordered_map<Vertex, uint32_t> uniqueVerties;

		// process mesh
		for (tinyobj::shape_t& s : shapes)
		{
			std::vector<Vertex> verts;
			std::vector<uint32_t> _inds;
			for (tinyobj::index_t& index : s.mesh.indices)
			{
				Vertex vert = {};
				glm::vec3 pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				glm::vec3 nrm = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				glm::vec2 texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vert.pos = pos;
				vert.normal = nrm;
				vert.texCoord = texCoord;

				if (uniqueVerties[vert] == uint32_t(0))
				{
					uniqueVerties[vert] = static_cast<uint32_t>(verts.size());
					verts.push_back(vert);
				}

				_inds.push_back(uniqueVerties[vert]);
			}
			generateVertexTangent(verts, _inds);

			// process material
			Material mat;
			mat.m_albedoMap = texture_manager->GetDefault("albedo_map");
			mat.m_normalMap = texture_manager->GetDefault("normal_map");
			mat.m_specularMap = texture_manager->GetDefault("specular_map");
			int mat_id = s.mesh.material_ids[0];
			Ref<GPipeline> sp = pipeline_manager->GetPipeline("gbuffer_pipeline");
			tinyobj::material_t& material = materials[mat_id];
			Ref<MaterialInstance> _mi = material_manager->GetMaterial("default");

			if (mat_id >= 0)
			{

				if (!material.diffuse_texname.empty())
				{
					if (Ref<GTexture> albe = texture_manager->LoadTexture(material.diffuse_texname))
						mat.m_albedoMap = albe;
					else
						TRC_ERROR("Failed to load texture {}", material.diffuse_texname);
				}
				if (!material.specular_texname.empty())
				{
					if (Ref<GTexture> spec = texture_manager->LoadTexture(material.specular_texname))
						mat.m_specularMap = spec;
					else
						TRC_ERROR("Failed to load texture {}", material.specular_texname);
				}

				if (!material.bump_texname.empty())
				{
					TextureDesc texture_desc;
					texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
					texture_desc.m_format = Format::R8G8B8A8_UNORM;
					texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
					texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
					texture_desc.m_usage = UsageFlag::DEFAULT;
					if (Ref<GTexture> nrm = texture_manager->LoadTexture(material.bump_texname, texture_desc))
						mat.m_normalMap = nrm;
					else
						TRC_ERROR("Failed to load texture {}", material.bump_texname);
				}

				mat.m_shininess = material.shininess < 0.0f ? mat.m_shininess : material.shininess;
				mat.m_diffuseColor = glm::vec4(
					material.diffuse[0],
					material.diffuse[1],
					material.diffuse[2],
					1.0f
				);

				material_manager->CreateMaterial(
					material.name,
					mat,
					sp
				);

				_mi = material_manager->GetMaterial(material.name);

			}

			Ref<Model> _model = model_manager->LoadModel_(verts,_inds,(path / s.name).string());
			if (_model.is_valid())
			{
				_model->m_matInstance = _mi;
			}

			_mesh->GetModels().emplace_back(_model);
		}

		

		return result;
	}

}