#include "pch.h"

#include "MeshManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "PipelineManager.h"
#include "MaterialManager.h"
#include "OBJ_Loader.h"
#include "backends/Renderutils.h"
#include "scene/UUID.h"
#include "serialize/MaterialSerializer.h"

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

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

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
		if (m_meshes.empty())
			return;

		for (Mesh& _mesh : m_meshes)
		{
			if (_mesh.m_id != INVALID_ID)
			{
				Unload(&_mesh);
				TRC_TRACE("Mesh not released , name : {}, RefCount : {}", _mesh.GetName(), _mesh.m_refCount);
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

	Ref<Mesh> MeshManager::LoadMeshOnly_(const std::string& path)
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
			if (LoadMeshOnly__OBJ(p))
			{
				return GetMesh(name);
			}

		}

		return result;
	}

	void MeshManager::Unload(Resource* res)
	{
		Mesh* _mesh = (Mesh*)res;
		if (_mesh->m_refCount > 0)
		{
			TRC_WARN("Current mesh is still in use can't unload");
			return;
		}



		_mesh->m_id = INVALID_ID;
		_mesh->~Mesh();
		_mesh->GetModels() = std::vector<Ref<Model>>();

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

	std::vector<std::string> MeshManager::GetAllModels(const std::string& path)
	{
		std::filesystem::path p(path);
		if (p.extension() == ".obj")
		{
			return GetAllModels_OBJ(p);
		}
		return std::vector<std::string>();
	}

	bool MeshManager::LoadDefaults()
	{

		std::vector<Vertex> verts;
		std::vector<uint32_t> _ind;
		Mesh* _mesh = nullptr;
		
		//Cube
		{
			generateDefaultCube(verts, _ind);
			generateVertexTangent(verts, _ind);
			Ref<Model> cube_ref = ModelManager::get_instance()->LoadModel(verts, _ind, "Cube");

			for (uint32_t k = 0; k < m_numEntries; k++)
			{
				if (m_meshes[k].m_id == INVALID_ID)
				{
					m_meshes[k].m_id = k;
					m_hashtable.Set("Cube", k);
					_mesh = &m_meshes[k];
					break;
				}
			}


			DefaultCube = { _mesh, BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, this) };
			_mesh = nullptr;
			DefaultCube->GetModels().push_back(cube_ref);
			DefaultCube->m_path = "Cube";
		}

		// Sphere
		{
			verts.clear();
			_ind.clear();

			generateSphere(verts, _ind, 1.0f, 45, 45);
			generateVertexTangent(verts, _ind);
			Ref<Model> sphere_ref = ModelManager::get_instance()->LoadModel(verts, _ind, "Sphere");
			


			for (uint32_t k = 0; k < m_numEntries; k++)
			{
				if (m_meshes[k].m_id == INVALID_ID)
				{
					m_meshes[k].m_id = k;
					m_hashtable.Set("Sphere", k);
					_mesh = &m_meshes[k];
					break;
				}
			}
			DefaultSphere = { _mesh, BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, this) };
			DefaultSphere->GetModels().push_back(sphere_ref);
			DefaultSphere->m_path = "Sphere";
		};

		//Plane
		{
			verts.clear();
			_ind.clear();

			generateDefaultPlane(verts, _ind);
			generateVertexTangent(verts, _ind);
			Ref<Model> plane_ref = ModelManager::get_instance()->LoadModel(verts, _ind, "Plane");


			for (uint32_t k = 0; k < m_numEntries; k++)
			{
				if (m_meshes[k].m_id == INVALID_ID)
				{
					m_meshes[k].m_id = k;
					m_hashtable.Set("Plane", k);
					_mesh = &m_meshes[k];
					break;
				}
			}
			DefaultPlane = { _mesh, BIND_RESOURCE_UNLOAD_FN(MeshManager::Unload, this) };
			DefaultPlane->GetModels().push_back(plane_ref);
			DefaultPlane->m_path = "Plane";
		};

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

	std::vector<std::string> MeshManager::GetAllModels_OBJ(std::filesystem::path& path)
	{
		std::vector<std::string> res;
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
			return res;
		}

		for (tinyobj::shape_t& s : shapes)
		{
			res.push_back(s.name);
		}

		return res;
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

				if (!obj_mesh.MeshMaterial.map_Kd.empty())
				{

					if (Ref<GTexture> albe = texture_manager->LoadTexture(obj_mesh.MeshMaterial.map_Kd))
					{
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
					}
					else
					{
						TRC_WARN("Failed to load texture {}", obj_mesh.MeshMaterial.map_bump);
					}
				}


				Ref<Model> _model = model_manager->LoadModel_(vert,_ind,( path / obj_mesh.MeshName).string());
				if (_model)
				{
					Ref<GPipeline> sp = pipeline_manager->GetPipeline("gbuffer_pipeline");
					Ref<MaterialInstance> _mi = material_manager->CreateMaterial(obj_mesh.MeshMaterial.name,sp);
					//_model->m_matInstance = _mi;
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
			Ref<Model> _model = model_manager->GetModel(s.name);
			if (_model) break;

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

			int mat_id = s.mesh.material_ids[0];
			Ref<GPipeline> sp = pipeline_manager->GetPipeline("gbuffer_pipeline");
			tinyobj::material_t& material = materials[mat_id];
			Ref<MaterialInstance> _mi = material_manager->GetMaterial("default");


			//New Way
			{

				if (mat_id >= 0)
				{
					UUID id = GetUUIDFromName(material.name + ".trmat");
					if (id != 0)
					{
						_mi = MaterialSerializer::Deserialize(GetPathFromUUID(id).string());
					}
					else
					{
						Ref<MaterialInstance> res = material_manager->CreateMaterial(material.name, sp);

						auto it1 = res->GetMaterialData().find("DIFFUSE_MAP");
						if (it1 != res->GetMaterialData().end())
						{
							if (!material.diffuse_texname.empty())
							{
								if (Ref<GTexture> albe = texture_manager->LoadTexture(material.diffuse_texname))
									it1->second.first = albe;
								else
									TRC_ERROR("Failed to load texture {}", material.diffuse_texname);
							}
							else
							{
								it1->second.first = texture_manager->GetDefault("albedo_map");
							}
						}
						auto it2 = res->GetMaterialData().find("SPECULAR_MAP");
						if (it2 != res->GetMaterialData().end())
						{
							if (!material.specular_texname.empty())
							{
								if (Ref<GTexture> spec = texture_manager->LoadTexture(material.specular_texname))
									it2->second.first = spec;
								else
									TRC_ERROR("Failed to load texture {}", material.specular_texname);
							}
							else
							{
								it2->second.first = texture_manager->GetDefault("specular_map");
							}
						}
						auto it3 = res->GetMaterialData().find("NORMAL_MAP");
						if (it3 != res->GetMaterialData().end())
						{
							if (!material.bump_texname.empty())
							{
								if (Ref<GTexture> nrm = texture_manager->LoadTexture(material.bump_texname))
									it3->second.first = nrm;
								else
									TRC_ERROR("Failed to load texture {}", material.bump_texname);
							}
							else
							{
								it3->second.first = texture_manager->GetDefault("normal_map");
							}
						}
						auto it4 = res->GetMaterialData().find("diffuse_color");
						if (it4 != res->GetMaterialData().end())
						{
							it4->second.first = glm::vec4(
								material.diffuse[0],
								material.diffuse[1],
								material.diffuse[2],
								1.0f
							);
						}
						auto it5 = res->GetMaterialData().find("shininess");
						if (it5 != res->GetMaterialData().end())
						{
							it5->second.first = material.shininess <= 0.0f ? 32.0f : material.shininess;
						}
						_mi = res;
						RenderFunc::PostInitializeMaterial(_mi.get(), sp);
					}

					
				}
			};

			_model = model_manager->LoadModel_(verts,_inds,(path / s.name).string());
			if (_model.is_valid())
			{
				//_model->m_matInstance = _mi;
			}

			_mesh->GetModels().emplace_back(_model);
		}

		

		return result;
	}

	bool MeshManager::LoadMeshOnly__OBJ(std::filesystem::path& path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		bool result = true;
		std::string name = path.filename().string();
		result = tinyobj::LoadObj(
			&attrib,
			&shapes,
			&materials,
			&err,
			path.string().c_str(),
			nullptr
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
			Ref<Model> _model = model_manager->GetModel(s.name);
			if (_model)
			{
				_mesh->GetModels().emplace_back(_model);
				continue;
			}

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

			
			_model = model_manager->LoadModel_(verts, _inds, (path / s.name).string());
			_mesh->GetModels().emplace_back(_model);
		}



		return result;
	}

	void ImportOBJ(const std::string& path, std::vector<std::string>& out_models, bool create_materials)
	{
		std::filesystem::path p(path);
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string parent_path = (p.parent_path() / "").generic_string();

		bool result = true;
		std::string name = p.filename().string();
		result = tinyobj::LoadObj(
			&attrib,
			&shapes,
			&materials,
			&err,
			p.string().c_str(),
			parent_path.c_str()
		);

		if (!result)
		{
			TRC_ERROR("Failed to load mesh {}", name);
			TRC_ERROR(err);
			return;
		}

		for (tinyobj::shape_t& s : shapes)
		{
			out_models.push_back(s.name);
			if (create_materials)
			{
				int mat_id = s.mesh.material_ids[0];
				tinyobj::material_t& material = materials[mat_id];

				if (mat_id >= 0)
				{
					std::filesystem::path mat_path = p.parent_path() / (material.name + ".trmat");
					if (std::filesystem::exists(mat_path)) continue;

					TextureManager* texture_manager = TextureManager::get_instance();
					PipelineManager* pipeline_manager = PipelineManager::get_instance();
					MaterialManager* material_manager = MaterialManager::get_instance();

					Ref<GPipeline> sp = pipeline_manager->GetPipeline("gbuffer_pipeline");
					Ref<MaterialInstance> res = material_manager->CreateMaterial(
						material.name + ".trmat",
						sp
					);
					GTexture diffuse;
					GTexture specular;
					GTexture normal;

					Ref<GTexture> diffuse_ref(&diffuse, [](Resource*) {});
					Ref<GTexture> specular_ref(&specular, [](Resource*) {});
					Ref<GTexture> normal_ref(&normal, [](Resource*) {});

					auto it1 = res->GetMaterialData().find("DIFFUSE_MAP");
					if (it1 != res->GetMaterialData().end())
					{
						if (!material.diffuse_texname.empty())
						{
							std::filesystem::path tex_p = material.diffuse_texname;
							diffuse_ref->m_path = GetPathFromUUID(GetUUIDFromName(tex_p.filename().string()));
							it1->second.first = diffuse_ref;
						}
						else
						{
							it1->second.first = texture_manager->GetDefault("albedo_map");
						}
					}
					auto it2 = res->GetMaterialData().find("SPECULAR_MAP");
					if (it2 != res->GetMaterialData().end())
					{
						if (!material.specular_texname.empty())
						{
							std::filesystem::path tex_p = material.specular_texname;
							specular_ref->m_path = GetPathFromUUID(GetUUIDFromName(tex_p.filename().string()));
							it2->second.first = specular_ref;
						}
						else
						{
							it2->second.first = texture_manager->GetDefault("specular_map");
						}
					}
					auto it3 = res->GetMaterialData().find("NORMAL_MAP");
					if (it3 != res->GetMaterialData().end())
					{
						if (!material.bump_texname.empty())
						{
							std::filesystem::path tex_p = material.bump_texname;
							normal_ref->m_path = GetPathFromUUID(GetUUIDFromName(tex_p.filename().string()));
							it3->second.first = normal_ref;
						}
						else
						{
							it3->second.first = texture_manager->GetDefault("normal_map");
						}
					}
					auto it4 = res->GetMaterialData().find("diffuse_color");
					if (it4 != res->GetMaterialData().end())
					{
						it4->second.first = glm::vec4(
							material.diffuse[0],
							material.diffuse[1],
							material.diffuse[2],
							1.0f
						);
					}
					auto it5 = res->GetMaterialData().find("shininess");
					if (it5 != res->GetMaterialData().end())
					{
						it5->second.first = material.shininess <= 0.0f ? 32.0f : material.shininess;
					}

					MaterialSerializer::Serialize(res, mat_path.string());

					res.release();
					res.free();
				}
			}
		}



	}

}