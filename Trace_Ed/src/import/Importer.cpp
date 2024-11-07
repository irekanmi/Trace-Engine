
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
#include "resource/GenericAssetManager.h"
#include "serialize/AnimationsSerializer.h"
#include "resource/AnimationsManager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#include "stb_image.h"
#include <filesystem>
#include <unordered_map>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"




#define ASSIMP_LOAD_FLAGS aiProcess_Triangulate

namespace trace {

	glm::mat4 aiMatrixToGlm(aiMatrix4x4 val)
	{
		glm::mat4 result;
		result[0][0] = val.a1; result[0][1] = val.b1; result[0][2] = val.c1; result[0][3] = val.d1;
		result[1][0] = val.a2; result[1][1] = val.b2; result[1][2] = val.c2; result[1][3] = val.d2;
		result[2][0] = val.a3; result[2][1] = val.b3; result[2][2] = val.c3; result[2][3] = val.d3;
		result[3][0] = val.a4; result[3][1] = val.b4; result[3][2] = val.c4; result[3][3] = val.d4;

		return result;
	}

	glm::vec3 aiVec3ToGlm(aiVector3D val)
	{
		return glm::vec3(val.x, val.y, val.z);
	}
	
	glm::quat aiQuatToGlm(aiQuaternion val)
	{
		return glm::quat(val.w, val.x, val.y, val.z);
	}

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

	void load_material(const aiScene* result, Ref<MaterialInstance> material, aiMaterial* imp_material, std::string& filename, std::filesystem::path& directory)
	{
		TextureManager* texture_manager = TextureManager::get_instance();

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

	Ref<SkinnedModel> load_assimp_skinned_mesh(const aiScene* scene, aiMesh* mesh, const std::string& model_name, std::string& filename, Ref<Skeleton>& out_skeleton_name, Importer* importer)
	{
		Ref<SkinnedModel> result;


		std::vector<SkinnedVertex> vertices;
		vertices.reserve(mesh->mNumVertices);
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			SkinnedVertex vert;
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

		auto lambda = [importer, &filename, &out_skeleton_name](aiBone* bone)->int32_t
		{
			for (auto& skeleton : importer->GetImportedSkeletons()[filename])
			{
				int32_t bone_index = skeleton->GetBoneIndex(bone->mName.C_Str());
				if (bone_index >= 0)
				{
					out_skeleton_name = skeleton;
					return bone_index;
				}
			}
			return -2;
		};

		for (uint32_t i = 0; i < mesh->mNumBones; i++)
		{
		
			aiBone* bone = mesh->mBones[i];
			int32_t bone_index = lambda(bone);
			for (uint32_t j = 0; j < bone->mNumWeights; j++)
			{
				aiVertexWeight weight = bone->mWeights[j];
				SkinnedVertex& vert = vertices[weight.mVertexId];
				for (uint32_t k = 0; k < MAX_BONE_PER_VERTEX; k++)
				{
					if (vert.bone_weights[k] == 0.0f)
					{
						vert.bone_weights[k] = weight.mWeight;
						vert.bones_id[k] = bone_index;
						break;
					}
				}
			}
		}

		generateVertexTangent(vertices, indices);
		result = GenericAssetManager::get_instance()->CreateAssetHandle_<SkinnedModel>(model_name);
		result->Init(vertices, indices);

		return result;
	}

	bool load_assimp_node_meshes(const aiScene* ass_scene, aiNode* node, Ref<Scene> scene, Entity parent, std::string& filename, std::filesystem::path& directory, Importer* importer)
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

			if (mesh->HasBones())
			{
				SkinnedModelRenderer& model_renderer = object.AddComponent<SkinnedModelRenderer>();
				Ref<Skeleton> skeleton;
				model_renderer._model = load_assimp_skinned_mesh(ass_scene, mesh, model_name, filename, skeleton, importer);
				model_renderer.SetSkeleton(skeleton, scene.get(), object.GetID());
				
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

						MaterialManager::get_instance()->RecreateMaterial(material, PipelineManager::get_instance()->GetPipeline("skinned_gbuffer_pipeline"));

						load_material(ass_scene, material, ass_mat, filename, directory);
						
						MaterialSerializer::Serialize(material, material_path.string());
						content_browser->ProcessAllDirectory(true);
						model_renderer._material = material;
					}

				}

			}
			else
			{
				
				object.AddComponent<ModelComponent>()._model = load_assimp_mesh(ass_scene, mesh, model_name);

				//content_browser->GetAllFilesID()[model_name] = UUID::GenUUID();

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
		}

		return true;
	}

	void process_assimp_node(const aiScene* ass_scene, aiNode* node, Ref<Scene> scene, Entity parent,std::string& filename, std::filesystem::path& directory, Importer* importer)
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
		load_assimp_node_meshes(ass_scene, node, scene, object, filename, directory, importer);

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			aiNode* chid_node = node->mChildren[i];
			process_assimp_node(ass_scene, chid_node, scene, object, filename, directory, importer);
		}
	}


	void import_materials(const aiScene* ass_scene, std::string& filename, std::filesystem::path& directory)
	{
		TextureManager* texture_manager = TextureManager::get_instance();

		const aiScene* result = ass_scene;

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


			load_material(result, material, imp_material, filename, directory);
			

			MaterialSerializer::Serialize(material, material_path);
		}
	}

	void add_bone(aiNode* node, std::unordered_map<std::string, Bone>& bones_map, std::vector<Bone>& bones)
	{
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			aiNode* child = node->mChildren[i];
			auto it = bones_map.find(child->mName.C_Str());
			if (it != bones_map.end())
			{
				bones.push_back(it->second);
			}
			add_bone(child, bones_map, bones);
		}
	}

	void create_skeleton(const aiScene* ass_scene, aiNode* node, std::unordered_map<std::string, Bone>& bones_map, std::string& root_node, std::string& filename, std::filesystem::path& directory, Importer* importer)
	{
		std::vector<Bone> bones;
		bones.push_back(bones_map[node->mName.C_Str()]);
		add_bone(node, bones_map, bones);
		std::string import_filename = std::filesystem::path(filename).stem().string();

		std::string skeleton_name = (root_node == "RootNode") ? import_filename + ".trcsk" : root_node + ".trcsk";
		root_node = (root_node == "RootNode") ? import_filename : root_node;

		Ref<Skeleton> skeleton;
		if (std::filesystem::exists(directory / skeleton_name))
		{
			skeleton = AnimationsSerializer::DeserializeSkeleton((directory / skeleton_name).string());
		}
		else
		{
			skeleton = GenericAssetManager::get_instance()->CreateAssetHandle_<Skeleton>((directory / skeleton_name).string());
			skeleton->Create(skeleton_name, root_node, bones);

			TraceEditor* editor = TraceEditor::get_instance();
			ContentBrowser* content_browser = editor->GetContentBrowser();

			AnimationsSerializer::SerializeSkeleton(skeleton, (directory / skeleton_name).string());
			content_browser->ProcessAllDirectory(true);
		}

		if (skeleton)
		{
			importer->GetImportedSkeletons()[filename].push_back(skeleton);
		}


	}

	void find_and_create_skeletons(const aiScene* ass_scene, aiNode* node, std::unordered_map<std::string, Bone>& bones_map, std::string& filename, std::filesystem::path& directory, Importer* importer)
	{
		std::string root_node;
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			aiNode* child = node->mChildren[i];
			auto it = bones_map.find(child->mName.C_Str());
			if (it != bones_map.end())
			{
				root_node = node->mName.C_Str();
				create_skeleton(ass_scene, child, bones_map, root_node, filename, directory, importer);
				continue;
			}
			find_and_create_skeletons(ass_scene, child, bones_map, filename, directory, importer);
		}
	}

	void import_skeletons(const aiScene* ass_scene, std::string& filename, std::filesystem::path& directory, Importer* importer)
	{
		/*for (uint32_t i = 0; i < ass_scene->mNumSkeletons; i++)
		{
			aiSkeleton* ass_skeleton = ass_scene->mSkeletons[i];
			std::string skeleton_name = ass_skeleton->mName.C_Str();
			skeleton_name += ".trcsk";

			std::vector<Bone> bones;
			for (uint32_t j = 0; j < ass_skeleton->mNumBones; j++)
			{
				aiSkeletonBone* ass_bone = ass_skeleton->mBones[j];
				std::string bone_name = ass_bone->mNode->mName.C_Str();
				glm::mat4 bind_pose = aiMatrixToGlm(ass_bone->mLocalMatrix);
				glm::mat4 bone_offset = aiMatrixToGlm(ass_bone->mOffsetMatrix);
				Bone bone;
				bone.Create(bone_name, bind_pose, bone_offset);

				bones.push_back(bone);
			}

			Ref<Skeleton> skeleton = GenericAssetManager::get_instance()->CreateAssetHandle_<Skeleton>((directory / skeleton_name).string());
			skeleton->Create(skeleton_name, bones);

			AnimationsSerializer::SerializeSkeleton(skeleton, (directory / skeleton_name).string());
		}*/

		std::unordered_map<std::string, Bone> bones_map;

		for (uint32_t i = 0; i < ass_scene->mNumMeshes; i++)
		{
			aiMesh* mesh = ass_scene->mMeshes[i];
			for (uint32_t j = 0; j < mesh->mNumBones; j++)
			{
				aiBone* ass_bone = mesh->mBones[j];
				std::string bone_name = ass_bone->mName.C_Str();
				glm::mat4 bind_pose = glm::mat4(1.0f);

				aiVector3D pos;
				aiVector3D scl;
				aiQuaternion rot;

				ass_bone->mOffsetMatrix.Decompose(scl, rot, pos);

				glm::vec3 position(pos.x, pos.y, pos.z);
				glm::vec3 scale(scl.x, scl.y, scl.z);
				glm::quat rotation(rot.w, rot.x, rot.y, rot.z);

				glm::mat4 bone_offset = glm::mat4(1.0f);
				bone_offset = glm::translate(bone_offset, position);
				bone_offset *= glm::toMat4(rotation);
				bone_offset = glm::scale(bone_offset, scale);
				Bone bone;
				bone.Create(bone_name, bind_pose, bone_offset);
				bones_map[bone_name] = bone;
			}
		}

		//Create Skeleton
		find_and_create_skeletons(ass_scene, ass_scene->mRootNode, bones_map, filename, directory, importer);
		
	}

	void import_animations(const aiScene* ass_scene, std::string& filename, std::filesystem::path& directory, Importer* importer)
	{

		for (uint32_t i = 0; i < ass_scene->mNumAnimations; i++)
		{
			aiAnimation* animation = ass_scene->mAnimations[i];

			std::string anim_name = animation->mName.C_Str();
			anim_name += ".trcac";

			float duration = (float)animation->mDuration / (float)animation->mTicksPerSecond;
			int frames_per_second = (int)animation->mTicksPerSecond;

			Ref<AnimationClip> clip = AnimationsManager::get_instance()->CreateClip(anim_name);
			clip->SetDuration(duration);
			clip->SetSampleRate(frames_per_second);
			auto& tracks = clip->GetTracks();


			for (uint32_t j = 0; j < animation->mNumChannels; j++)
			{
				aiNodeAnim* node_anim = animation->mChannels[j];
				std::string node_name = node_anim->mNodeName.C_Str();

				AnimationTrack position_track;
				position_track.channel_type = AnimationDataType::POSITION;
				
				for (uint32_t k = 0; k < node_anim->mNumPositionKeys; k++)
				{
					aiVectorKey key = node_anim->mPositionKeys[k];
					AnimationFrameData frame_data;
					frame_data.time_point = duration * (float)(key.mTime / animation->mDuration);					

					glm::vec3 val = aiVec3ToGlm(key.mValue);
					memcpy(frame_data.data, &val, sizeof(glm::vec3));

					position_track.channel_data.emplace_back(frame_data);
				}

				AnimationTrack rotation_track;
				rotation_track.channel_type = AnimationDataType::ROTATION;

				for (uint32_t k = 0; k < node_anim->mNumRotationKeys; k++)
				{
					aiQuatKey key = node_anim->mRotationKeys[k];
					AnimationFrameData frame_data;
					frame_data.time_point = duration * (float)(key.mTime / animation->mDuration);

					glm::quat val = aiQuatToGlm(key.mValue);
					memcpy(frame_data.data, &val, sizeof(glm::quat));

					rotation_track.channel_data.emplace_back(frame_data);
				}

				AnimationTrack scale_track;
				scale_track.channel_type = AnimationDataType::SCALE;

				for (uint32_t k = 0; k < node_anim->mNumScalingKeys; k++)
				{
					aiVectorKey key = node_anim->mScalingKeys[k];
					AnimationFrameData frame_data;
					frame_data.time_point = duration * (float)(key.mTime / animation->mDuration);

					glm::vec3 val = aiVec3ToGlm(key.mValue);
					memcpy(frame_data.data, &val, sizeof(glm::vec3));

					scale_track.channel_data.emplace_back(frame_data);
				}

				std::vector<AnimationTrack> node_tracks;
				node_tracks.emplace_back(position_track);
				node_tracks.emplace_back(rotation_track);
				node_tracks.emplace_back(scale_track);

				//tracks[node_name] = std::move(node_tracks);
				


			}

			TraceEditor* editor = TraceEditor::get_instance();
			ContentBrowser* content_browser = editor->GetContentBrowser();

			AnimationsSerializer::SerializeAnimationClip(clip, (directory / anim_name).string());
			content_browser->ProcessAllDirectory(true);
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

		m_importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		const aiScene* result = m_importer.ReadFile(file_path, ASSIMP_LOAD_FLAGS);

		if (!result)
		{
			TRC_ERROR("Failed to import mesh, filename: {}", file_path);
			return false;
		}

		m_loadedFiles[filename] = result;
		content_browser->ProcessAllDirectory();// Refresh file id's

		// Import materials ---------------------------------------------------------
		import_materials(result, filename, directory);
		// -----------------------------------------------------------

		// Import skeletons ---------------------------------------------------------
		import_skeletons(result, filename, directory, this);
		// -----------------------------------------------------------

		// Import animations ---------------------------------------------------------
		import_animations(result, filename, directory, this);
		// -----------------------------------------------------------

		content_browser->ProcessAllDirectory(true);// Refresh file id's

		// Import Meshes
		static int scene_count = 0;
		Ref<Scene> temp_scene = SceneManager::get_instance()->CreateScene("Import Scene" + std::to_string(scene_count));
		scene_count++;
		aiNode* root_node = result->mRootNode;
		Entity holder = temp_scene->CreateEntity();
		process_assimp_node(result, root_node, temp_scene, holder, filename, directory, this);


		content_browser->ProcessAllDirectory(true);// Refresh file id's
		
		HierachyComponent& hi = holder.GetComponent<HierachyComponent>();


		Entity root_entity = temp_scene->GetEntity(hi.children[0]);
		root_entity.GetComponent<TagComponent>().SetTag(import_filename);
		std::string prefab_name = root_entity.GetComponent<TagComponent>().GetTag() + ".trprf";
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
		m_importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);


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
	Ref<SkinnedModel> Importer::LoadSkinnedModel(const std::string& file_path)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		m_importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);


		Ref<SkinnedModel> model;

		std::vector<std::string> paths = SplitString(file_path, '`');

		const aiScene* scene = nullptr;

		std::filesystem::path directory;
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
			directory = dir_path.parent_path();

			if (!result)
			{
				TRC_ERROR("Failed to open mesh, filename: {}", paths[0]);
				return model;
			}

			import_skeletons(result, paths[0], directory, this);
			m_loadedFiles[paths[0]] = result;

			scene = result;

		}

		for (uint32_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];
			std::string mesh_name = mesh->mName.C_Str();
			if (mesh_name == paths[1])
			{
				Ref<Skeleton> skeleton;
				model = load_assimp_skinned_mesh(scene, mesh,file_path, paths[0], skeleton, this);
				break;
			}
		}

		return model;
	}
}
