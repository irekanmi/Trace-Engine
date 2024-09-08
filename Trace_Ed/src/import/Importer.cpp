
#include <vector>
#include <string>
#include "Importer.h"
#include "../TraceEditor.h"
#include "../panels/ContentBrowser.h"
#include "serialize/MaterialSerializer.h"
#include "resource/MaterialManager.h"
#include "resource/PipelineManager.h"
#include "resource/TextureManager.h"
#include "resource/ModelManager.h"
#include "render/Material.h"
#include "resource/Ref.h"
#include "resource/PrefabManager.h"
#include "resource/Prefab.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "scene/SceneManager.h"
#include "serialize/SceneSerializer.h"
#include "core/Utils.h"
#include "render/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#include "stb_image.h"
#include <filesystem>


#define ASSIMP_LOAD_FLAGS aiProcess_Triangulate

namespace trace {

	Ref<GTexture> load_assimp_embedded_texture(const aiScene* scene, std::string& filename, const std::string& texture_path, bool generate_uuid)
	{
		Ref<GTexture> result;
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		TextureManager* texture_manager = TextureManager::get_instance();

		const aiTexture* ass_texture = scene->GetEmbeddedTexture(texture_path.c_str());

		if (ass_texture)
		{
			TextureDesc texture_desc;
			texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
			texture_desc.m_channels = 4;
			texture_desc.m_format = Format::R8G8B8A8_UNORM;
			texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
			texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
			texture_desc.m_usage = UsageFlag::DEFAULT;
			texture_desc.m_image_type = ImageType::IMAGE_2D;
			texture_desc.m_numLayers = 1;

			std::string tex_name = filename + "`" + texture_path;
			if (ass_texture->mHeight == 0)
			{
				int _width, _height, _channels;
				unsigned char* pixel_data = nullptr;

				stbi_set_flip_vertically_on_load(true);
				pixel_data = stbi_load_from_memory((unsigned char*)ass_texture->pcData, ass_texture->mWidth, &_width, &_height, &_channels, STBI_rgb_alpha);
				texture_desc.m_width = (uint32_t)_width;
				texture_desc.m_height = (uint32_t)_height;
				texture_desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height))) / 2) + 1;
				texture_desc.m_data.push_back(pixel_data);
				Ref<GTexture> texture = texture_manager->CreateTexture(tex_name, texture_desc);
				if (texture)
				{
					result = texture;
				}
				if (generate_uuid)
				{
					content_browser->GetAllFilesID()[tex_name] = UUID::GenUUID();
				}
			}
			else
			{
				texture_desc.m_width = (uint32_t)ass_texture->mWidth;
				texture_desc.m_height = (uint32_t)ass_texture->mHeight;
				texture_desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture_desc.m_width, texture_desc.m_height))) / 2) + 1;
				texture_desc.m_data.push_back(((unsigned char*)ass_texture->pcData));


				Ref<GTexture> texture = texture_manager->CreateTexture(tex_name, texture_desc);
				if (texture)
				{
					result = texture;
				}
				if (generate_uuid)
				{
					content_browser->GetAllFilesID()[tex_name] = UUID::GenUUID();
				}

			}
		}

		return result;
	}

	Ref<GTexture> load_assimp_texure(const aiScene* scene, aiMaterial* material, aiTextureType tex_type, std::filesystem::path& directory, std::string& filename)
	{
		Ref<GTexture> result;

		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		TextureManager* texture_manager = TextureManager::get_instance();

		aiString texture_path;
		int texture_index = 0;
		if (aiGetMaterialTexture(material, tex_type, texture_index, &texture_path) == AI_SUCCESS)
		{
			const aiTexture* ass_texture = scene->GetEmbeddedTexture(texture_path.C_Str());

			if (ass_texture)
			{
				
				std::string tex_path = texture_path.C_Str();
				load_assimp_embedded_texture(scene, filename, tex_path, true);
			}
			else
			{
				std::string texture_file_path = (directory / texture_path.C_Str()).string();
				Ref<GTexture> texture = texture_manager->LoadTexture_(texture_file_path);
				if (texture)
				{
					result = texture;
				}
			}

		}

		return result;
	}
	
	Ref<Model> load_assimp_mesh(const aiScene* scene, aiMesh* mesh, const std::string& model_name)
	{
		Ref<Model> result;


		std::vector<Vertex> vertices;
		vertices.reserve(mesh->mNumVertices);
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vert;
			aiVector3D position = mesh->mVertices[i];
			if (mesh->mNormals)
			{
				aiVector3D normal = mesh->mNormals[i];
				vert.normal = glm::vec3(normal.x, normal.y, normal.z);
			}
			//aiVector3D tangent = mesh->mTangents[i];
			if (mesh->mTextureCoords[0])
			{
				aiVector3D tex_coord = mesh->mTextureCoords[0][i];

				vert.texCoord = glm::vec2(tex_coord.x, tex_coord.y);
			}
			else
			{

				vert.texCoord = glm::vec2(0.0f, 1.0f);
			}
			vert.pos = glm::vec3(position.x, position.y, position.z);
			//vert.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, 1.0f);

			vertices.push_back(vert);

		}

		std::vector<uint32_t> indices;
		indices.reserve(mesh->mNumFaces * 3);
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < 3; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		generateVertexTangent(vertices, indices);
		result = ModelManager::get_instance()->LoadModel(vertices, indices, model_name);

		return result;
	}

	bool load_assimp_node_meshes(const aiScene* ass_scene, aiNode* node, Ref<Scene> scene, Entity parent, const std::string& filename)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();

		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{

			uint32_t mesh_index = node->mMeshes[i];
			aiMesh* mesh = ass_scene->mMeshes[mesh_index];

			std::string mesh_name = mesh->mName.C_Str();

			UUID parent_id = 0;
			if (parent)
			{
				parent_id = parent.GetID();
			}

			Entity object = scene->CreateEntity(mesh_name, parent_id);
			
			std::string model_name = filename + "`" + mesh->mName.C_Str();
			object.AddComponent<ModelComponent>()._model = load_assimp_mesh(ass_scene, mesh, model_name);

			content_browser->GetAllFilesID()[model_name] = UUID::GenUUID();

			if (mesh->mMaterialIndex != -1)
			{
				aiMaterial* ass_mat = ass_scene->mMaterials[mesh->mMaterialIndex];
				std::string mat_name;
				aiString ass_name;
				if (aiGetMaterialString(ass_mat, AI_MATKEY_NAME, &ass_name) == AI_SUCCESS)
				{
					mat_name = ass_name.C_Str();
					mat_name += ".trmat";

					
				}
				else
				{
					mat_name = filename + "Unnamed_material" + std::to_string(mesh->mMaterialIndex);
					mat_name += ".trmat";
				}

				if (!mat_name.empty())
				{
					UUID id = content_browser->GetAllFilesID()[mat_name];
					std::filesystem::path material_path = content_browser->GetUUIDPath()[id];

					Ref<MaterialInstance> material = MaterialSerializer::Deserialize(material_path.string());

					object.AddComponent<ModelRendererComponent>()._material = material;
				}

			}
		}

		return true;
	}

	void process_assimp_node(const aiScene* ass_scene, aiNode* node, Ref<Scene> scene, Entity parent,const std::string& filename)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();

		std::string name = node->mName.C_Str();
		UUID parent_id = 0;
		if (parent)
		{
			parent_id = parent.GetID();
			TRC_INFO("Parent ID: {}", parent.GetID());
		}

		Entity object = scene->CreateEntity(name, parent_id);
		TRC_INFO("Node ID: {}", object.GetID());

		aiVector3D pos;
		aiVector3D scl;
		aiQuaternion rot;

		node->mTransformation.Decompose(scl, rot, pos);

		glm::vec3 position(pos.x, pos.y, pos.z);
		glm::vec3 scale(scl.x, scl.y, scl.z);
		glm::quat rotation(rot.w, rot.x, rot.y, rot.z);

		object.GetComponent<TransformComponent>()._transform.SetPosition(position);
		object.GetComponent<TransformComponent>()._transform.SetScale(scale);
		object.GetComponent<TransformComponent>()._transform.SetRotation(rotation);

		// NOTE: For now we load only one mesh
		/*if (node->mNumMeshes > 0)
		{
			uint32_t mesh_index = node->mMeshes[0];
			aiMesh* mesh = ass_scene->mMeshes[mesh_index];			
			
			std::string model_name = filename + "`" + mesh->mName.C_Str();
			object.AddComponent<ModelComponent>()._model = load_assimp_mesh(ass_scene, mesh, model_name);

			content_browser->GetAllFilesID()[model_name] = UUID::GenUUID();

			if (mesh->mMaterialIndex != -1)
			{
				aiMaterial* ass_mat = ass_scene->mMaterials[mesh->mMaterialIndex];
				aiString ass_name;
				if (aiGetMaterialString(ass_mat, AI_MATKEY_NAME, &ass_name) == AI_SUCCESS)
				{
					std::string mat_name = ass_name.C_Str();
					mat_name += ".trmat";

					UUID id = content_browser->GetAllFilesID()[mat_name];
					std::filesystem::path material_path = content_browser->GetUUIDPath()[id];

					Ref<MaterialInstance> material = MaterialSerializer::Deserialize(material_path.string());

					object.AddComponent<ModelRendererComponent>()._material = material;
				}

			}

		}*/
		load_assimp_node_meshes(ass_scene, node, scene, object, filename);

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			aiNode* chid_node = node->mChildren[i];
			process_assimp_node(ass_scene, chid_node, scene, object, filename);
		}
	}

	bool Importer::ImportMeshFile(const std::string& file_path)
	{
		std::filesystem::path path(file_path);
		std::filesystem::path directory = path.parent_path();
		std::string filename = path.filename().string();

		std::string import_filename = path.filename().stem().string();

		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();


		const aiScene* result = m_importer.ReadFile(file_path, ASSIMP_LOAD_FLAGS);

		if (!result)
		{
			TRC_ERROR("Failed to import mesh, filename: {}", file_path);
			return false;
		}

		m_loadedFiles[path.filename().string()] = result;
		content_browser->ProcessAllDirectory();// Refresh file id's

		// Import materials ---------------------------------------------------------
		TextureManager* texture_manager = TextureManager::get_instance();
				

		for (uint32_t i = 0; i < result->mNumMaterials; i++)
		{
			aiMaterial* imp_material = result->mMaterials[i];
			aiString mat_name;
			std::string material_name;
			if (imp_material->Get(AI_MATKEY_NAME, mat_name) == AI_SUCCESS)
			{
				material_name = mat_name.C_Str();
			}
			else
			{
				material_name = filename + "Unnamed_material" + std::to_string(i);
			}
			material_name += ".trmat";
			std::string material_path = (directory / material_name).string();
			Ref<MaterialInstance> material = MaterialManager::get_instance()->CreateMaterial(material_name, PipelineManager::get_instance()->GetPipeline("gbuffer_pipeline"));

			

			{
				auto it = material->GetMaterialData().find("emissive_color");
				if (it != material->GetMaterialData().end())
				{
					it->second.first = glm::vec4(0.0f);
					aiColor4D color;
					if (imp_material->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
					{
						glm::vec4 emissive_color;
						emissive_color.r = color.r;
						emissive_color.g = color.g;
						emissive_color.b = color.b;
						emissive_color.a = color.a;

						it->second.first = emissive_color;
					}
				}
			};

			{
				auto it = material->GetMaterialData().find("tilling");
				if (it != material->GetMaterialData().end())
				{
					it->second.first = glm::vec2(1.0f);
					
				}
			};

			

			{
				auto roughness_map_it = material->GetMaterialData().find("ROUGHNESS_MAP");
				if (roughness_map_it != material->GetMaterialData().end())
				{
					roughness_map_it->second.first = texture_manager->GetTexture("black_texture");

					Ref<GTexture> texture = load_assimp_texure(result, imp_material, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, directory, filename);
					if (texture)
					{
						roughness_map_it->second.first = texture;
					}
					else
					{
						auto it = material->GetMaterialData().find("roughness");
						if (it != material->GetMaterialData().end())
						{
							float roughness = 0.05f;
							aiGetMaterialFloat(imp_material, AI_MATKEY_ROUGHNESS_FACTOR, &roughness);

							float factor = 1.0f;


							it->second.first = glm::vec2(roughness, factor);
						}
					};
				}
			};

			

			{
				auto metallic_it = material->GetMaterialData().find("METALLIC_MAP");
				if (metallic_it != material->GetMaterialData().end())
				{
					metallic_it->second.first = texture_manager->GetTexture("black_texture");

					Ref<GTexture> texture = load_assimp_texure(result, imp_material, aiTextureType::aiTextureType_METALNESS, directory, filename);
					if (texture)
					{
						metallic_it->second.first = texture;
					}
					else
					{
						auto it = material->GetMaterialData().find("metallic");
						if (it != material->GetMaterialData().end())
						{
							float metallic = 0.1f;
							aiGetMaterialFloat(imp_material, AI_MATKEY_METALLIC_FACTOR, &metallic);
							float factor = 1.0f;


							it->second.first = glm::vec2(metallic, factor);
						}
					};
				}
			};

			{
				auto diffuse_map_it = material->GetMaterialData().find("DIFFUSE_MAP");
				if (diffuse_map_it != material->GetMaterialData().end())
				{
					diffuse_map_it->second.first = texture_manager->GetDefault("albedo_map");
					
					Ref<GTexture> texture = load_assimp_texure(result, imp_material, aiTextureType::aiTextureType_DIFFUSE, directory, filename);
					if (texture)
					{
						diffuse_map_it->second.first = texture;
					}
					else
					{
						auto it = material->GetMaterialData().find("diffuse_color");
						if (it != material->GetMaterialData().end())
						{
							it->second.first = glm::vec4(0.0f);
							aiColor4D color;
							if (imp_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
							{
								glm::vec4 diffuse_color;
								diffuse_color.r = color.r;
								diffuse_color.g = color.g;
								diffuse_color.b = color.b;
								diffuse_color.a = color.a;

								it->second.first = diffuse_color;
							}
						}
					};
				}
				else
				{
					auto it = material->GetMaterialData().find("diffuse_color");
					if (it != material->GetMaterialData().end())
					{
						it->second.first = glm::vec4(0.0f);
						aiColor4D color;
						if (imp_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
						{
							glm::vec4 diffuse_color;
							diffuse_color.r = color.r;
							diffuse_color.g = color.g;
							diffuse_color.b = color.b;
							diffuse_color.a = color.a;

							it->second.first = diffuse_color;
						}
					}
				}
				
			};

			{
				auto it = material->GetMaterialData().find("NORMAL_MAP");
				if (it != material->GetMaterialData().end())
				{
					it->second.first = texture_manager->GetDefault("normal_map");

					Ref<GTexture> texture = load_assimp_texure(result, imp_material, aiTextureType::aiTextureType_NORMALS, directory, filename);
					if (texture)
					{
						it->second.first = texture;
					}
				}
			};

			{
				auto it = material->GetMaterialData().find("height_scale");
				if (it != material->GetMaterialData().end())
				{
					float height_scale = 0.0f;
					if (aiGetMaterialFloat(imp_material, AI_MATKEY_BUMPSCALING, &height_scale) == AI_SUCCESS)
					{
						it->second.first = height_scale < 0.08f ? height_scale : 0.05f;

					}
					else
					{
						it->second.first = height_scale;
					}

				}
			};

			{
				auto it = material->GetMaterialData().find("HEIGHT_MAP");
				if (it != material->GetMaterialData().end())
				{
					it->second.first = texture_manager->GetTexture("black_texture");

					Ref<GTexture> texture = load_assimp_texure(result, imp_material, aiTextureType::aiTextureType_HEIGHT, directory, filename);
					if (texture)
					{
						it->second.first = texture;
					}
				}
			};


			{
				auto it = material->GetMaterialData().find("OCCLUSION_MAP");
				if (it != material->GetMaterialData().end())
				{
					it->second.first = texture_manager->GetDefault("albedo_map");

					Ref<GTexture> texture = load_assimp_texure(result, imp_material, aiTextureType::aiTextureType_AMBIENT_OCCLUSION, directory, filename);
					if (texture)
					{
						it->second.first = texture;
					}
				}
			};

			MaterialSerializer::Serialize(material, material_path);
		}

	// -----------------------------------------------------------

		content_browser->ProcessAllDirectory(true);// Refresh file id's

		// Import Meshes
		static int scene_count = 0;
		Ref<Scene> temp_scene = SceneManager::get_instance()->CreateScene("Import Scene" + std::to_string(scene_count));
		scene_count++;
		aiNode* root_node = result->mRootNode;
		Entity holder = temp_scene->CreateEntity();
		process_assimp_node(result, root_node, temp_scene, holder, filename);


		content_browser->ProcessAllDirectory(true);// Refresh file id's
		
		HierachyComponent& hi = holder.GetComponent<HierachyComponent>();


		Entity root_entity = temp_scene->GetEntity(hi.children[0]);
		root_entity.GetComponent<TagComponent>()._tag = import_filename;
		std::string prefab_name = root_entity.GetComponent<TagComponent>()._tag + ".trprf";
		Ref<Prefab> prefab = PrefabManager::get_instance()->Create(prefab_name, root_entity);
		SceneSerializer::SerializePrefab(prefab, (directory / prefab_name).string());

		content_browser->ProcessAllDirectory(true);// Refresh file id's

		return true;
	}
	Ref<GTexture> Importer::LoadTexture(const std::string& file_path)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();

		Ref<GTexture> texture;

		std::vector<std::string> paths = SplitString(file_path, '`');

		const aiScene* scene = nullptr;

		auto it = m_loadedFiles.find(paths[0]);
		if (it != m_loadedFiles.end())
		{
			scene = it->second;
		}
		else
		{
			UUID id = content_browser->GetAllFilesID()[paths[0]];
			std::filesystem::path dir_path = content_browser->GetUUIDPath()[id];
			const aiScene* result = m_importer.ReadFile(dir_path.string(), ASSIMP_LOAD_FLAGS);

			if (!result)
			{
				TRC_ERROR("Failed to open mesh, filename: {}", paths[0]);
				return texture;
			}

			m_loadedFiles[paths[0]] = result;

			scene = result;
			
		}

		texture = load_assimp_embedded_texture(scene, paths[0], paths[1], false);

		return texture;
	}
	Ref<Model> Importer::LoadModel(const std::string& file_path)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();

		Ref<Model> model;

		std::vector<std::string> paths = SplitString(file_path, '`');

		const aiScene* scene = nullptr;

		auto it = m_loadedFiles.find(paths[0]);
		if (it != m_loadedFiles.end())
		{
			scene = it->second;
		}
		else
		{
			UUID id = content_browser->GetAllFilesID()[paths[0]];
			std::filesystem::path dir_path = content_browser->GetUUIDPath()[id];
			const aiScene* result = m_importer.ReadFile(dir_path.string(), ASSIMP_LOAD_FLAGS);

			if (!result)
			{
				TRC_ERROR("Failed to open mesh, filename: {}", paths[0]);
				return model;
			}

			m_loadedFiles[paths[0]] = result;

			scene = result;

		}

		for (uint32_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];
			std::string mesh_name = mesh->mName.C_Str();
			if (mesh_name == paths[1])
			{
				model = load_assimp_mesh(scene, mesh, file_path);
				break;
			}
		}

		return model;
	}
}
