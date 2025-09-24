#include "pch.h"

#include "SceneSerializer.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "core/FileSystem.h"


#include "resource/GenericAssetManager.h"
#include "resource/PrefabManager.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "serialize/PipelineSerializer.h"
#include "serialize/GenericSerializer.h"
#include "scripting/ScriptEngine.h"
#include "backends/Renderutils.h"

#include "core/Coretypes.h"
#include "external_utils.h"
#include "animation/AnimationPose.h"
#include "animation/AnimationPoseNode.h"
#include "core/Utils.h"
#include "reflection/TypeHash.h"
#include "reflection/SerializeTypes.h"
#include "serialize/SceneSerializeFunctions.h"
#include "render/GShader.h"
#include "shader_graph/ShaderGraph.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>

#include "yaml_util.h"

namespace trace {


	
	bool SceneSerializer::Serialize(Ref<Scene> scene, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Scene Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Stimulate Physics" << YAML::Value << scene->GetStimulatePhysics();
		emit << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto process_hierachy = [&](Entity entity, UUID, Scene*)
		{
			Entity en(entity, scene.get());
			serialize_entity_components(en, emit, scene.get());
		};

		scene->ProcessEntitiesByHierachy(process_hierachy, false);

		emit << YAML::EndSeq;

		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, file_path);

		return true;
	}

	//TODO: Move serialization of an entity into a funtion
	static void SerializePrefabEntity(Entity entity, YAML::Emitter& emit)
	{

		serialize_entity_components(entity, emit, entity.GetScene());

		for(auto& i : entity.GetComponent<HierachyComponent>().children)
		{
			Entity child = entity.GetScene()->GetEntity(i);
			SerializePrefabEntity(child, emit);
		}

	}

	bool SceneSerializer::SerializePrefab(Ref<Prefab> prefab, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Prefab Version" << YAML::Value << "0.0.0.0";

		emit << YAML::Key << "Entity" << YAML::Value << YAML::BeginSeq;

		Scene* scene = PrefabManager::get_instance()->GetScene();
		Entity handle = scene->GetEntity(prefab->GetHandle());
		serialize_entity_components(handle, emit, handle.GetScene());

		emit << YAML::EndSeq;

		emit << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (auto& i : handle.GetComponent<HierachyComponent>().children)
		{
			Entity child = handle.GetScene()->GetEntity(i);
			SerializePrefabEntity(child, emit);
		}

		emit << YAML::EndSeq;

		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, file_path);

		return true;
	}

	//TODO: Move serialization of an entity into a funtion
	static void SerializePrefabEntity_Binary(Entity entity, DataStream* stream, uint32_t& entity_count)
	{

		serialize_entity_components_binary(entity, stream, entity.GetScene());

		for (auto& i : entity.GetComponent<HierachyComponent>().children)
		{
			Entity child = entity.GetScene()->GetEntity(i);
			SerializePrefabEntity_Binary(child, stream, entity_count);
			entity_count++;
		}

	}

	bool SceneSerializer::SerializePrefab(Ref<Prefab> prefab, DataStream* stream)
	{
		stream->Write<std::string>(prefab->GetName());

		Scene* scene = PrefabManager::get_instance()->GetScene();
		Entity handle = scene->GetEntity(prefab->GetHandle());
		
		SerializeEntity(handle, stream);

		return true;
	}

	Ref<Scene> SceneSerializer::Deserialize(const std::string& file_path)
	{
		YAML::Node data;
		YAML::load_yaml_data(file_path, data);
		if (!data["Trace Version"])
		{
			TRC_ERROR("These file is not a valid scene file {}", file_path);
			return Ref<Scene>();
		}

		std::filesystem::path p = file_path;

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string scene_version = data["Scene Version"].as<std::string>(); // TODO: To be used later
		std::string scene_name = p.filename().string();

		Ref<Scene> scene = GenericAssetManager::get_instance()->TryGet<Scene>(scene_name);
		if (scene)
		{
			TRC_WARN("{} has already been loaded", scene_name);
			return scene;
		}
		scene = GenericAssetManager::get_instance()->CreateAssetHandle<Scene>(scene_name);
		scene->SetName(scene_name);
		if (data["Stimulate Physics"])
		{
			scene->SetStimulatePhysics(data["Stimulate Physics"].as<bool>());
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				deserialize_entity_components(scene.get(), entity);
			}
		}

		if (AppSettings::is_editor)
		{
			scene->ApplyPrefabChangesOnSceneLoad();
		}

		scene->InitializeSceneComponents();

		return scene;
	}

	Ref<Prefab> SceneSerializer::DeserializePrefab(const std::string& file_path)
	{
		Ref<Prefab> result;

		YAML::Node data;
		YAML::load_yaml_data(file_path, data);
		if (!data["Trace Version"])
		{
			TRC_ERROR("These file is not a valid Prefab file {}", file_path);
			return result;
		}

		std::filesystem::path p = file_path;

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string prefab_version = data["Prefab Version"].as<std::string>(); // TODO: To be used later
		std::string prefab_name = p.filename().string();

		Ref<Prefab> prefab = GenericAssetManager::get_instance()->TryGet<Prefab>(prefab_name);
		if (prefab) return prefab;

		prefab = GenericAssetManager::get_instance()->CreateAssetHandle<Prefab>(prefab_name, nullptr);

		YAML::Node handle = data["Entity"];

		Scene* scene = PrefabManager::get_instance()->GetScene();

		Entity obj;

		if (handle)
		{
			for (auto entity : handle)
			{
				obj = deserialize_entity_components(scene, entity);
			}
		}

		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				deserialize_entity_components(scene, entity);
			}
		}

		prefab->SetHandle(obj.GetID());
		prefab->m_path = file_path;
		scene->EnableEntity(obj);

		result = prefab;

		return result;
	}

	Ref<Prefab> SceneSerializer::DeserializePrefab(DataStream* stream)
	{
		Ref<Prefab> result;

		std::string prefab_name;
		stream->Read<std::string>(prefab_name);
		Ref<Prefab> prefab = GenericAssetManager::get_instance()->TryGet<Prefab>(prefab_name);
		if (prefab)
		{
			return prefab;
		}

		prefab = GenericAssetManager::get_instance()->CreateAssetHandle<Prefab>(prefab_name, nullptr);


		Scene* scene = PrefabManager::get_instance()->GetScene();

		Entity obj;

		obj = DeserializeEntity(scene, stream);


		prefab->SetHandle(obj.GetID());

		result = prefab;

		return result;
	}

	bool SceneSerializer::Deserialize(Ref<Scene> scene, FileStream& stream, AssetHeader& header)
	{
		return true;
	}

	bool SceneSerializer::Serialize(Ref<Scene> scene, DataStream* stream)
	{
		stream->Write<std::string>(scene->GetName());
		bool stimulate_physics = scene->GetStimulatePhysics();
		stream->Write<bool>(stimulate_physics);

		uint32_t pos_1 = stream->GetPosition();
		uint32_t entity_count = scene->m_rootNode->children.size();
		stream->Write<uint32_t>(entity_count);

		for (UUID& i : scene->m_rootNode->children)
		{
			Entity en = scene->GetEntity(i);
			SerializeEntity(en, stream);
		}

		

		return true;
	}

	Ref<Scene> SceneSerializer::Deserialize(DataStream* stream)
	{
		std::string scene_name;
		stream->Read<std::string>(scene_name);

		bool stimulate_physics;
		stream->Read<bool>(stimulate_physics);

		Ref<Scene> scene = GenericAssetManager::get_instance()->TryGet<Scene>(scene_name);
		if (scene)
		{
			TRC_WARN("{} has already been loaded", scene_name);
			return scene;
		}
		scene = GenericAssetManager::get_instance()->CreateAssetHandle<Scene>(scene_name);
		scene->SetName(scene_name);
		scene->SetStimulatePhysics(stimulate_physics);

		uint32_t entity_count = 0;
		stream->Read<uint32_t>(entity_count);
		for (uint32_t i = 0; i < entity_count; i++)
		{
			DeserializeEntity(scene.get(), stream);
		}

		scene->InitializeSceneComponents();

		return scene;
	}

	/*
	* Texture
	*  '-> TextureDesc
	*  '-> uint32_t m_width = 0;
	*  '-> uint32_t m_height = 0;
	*  '-> uint32_t m_mipLevels = 1;
	*  '-> Format m_format = Format::NONE;
	*  '-> BindFlag m_flag = BindFlag::NIL;
	*  '-> UsageFlag m_usage = UsageFlag::NONE;
	*  '-> uint32_t m_channels = 0;
	*  '-> uint32_t m_numLayers = 0;
	*  '-> ImageType m_image_type = ImageType::NO_TYPE;
	*  '-> AddressMode m_addressModeU = AddressMode::NONE;
	*  '-> AddressMode m_addressModeV = AddressMode::NONE;
	*  '-> AddressMode m_addressModeW = AddressMode::NONE;
	*  '-> FilterMode m_minFilterMode = FilterMode::NONE;
	*  '-> FilterMode m_magFilterMode = FilterMode::NONE;
	*  '-> AttachmentType m_attachmentType = AttachmentType::NONE;
	*  '-> texture_data
	*/
	bool SceneSerializer::SerializeTextures(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		
		float total_tex_size = 0.0f;
		char* data = nullptr;// TODO: Use custom allocator
		uint32_t data_size = 0;

		auto serialize_tex = [&](Ref<GTexture> tex)
		{
			UUID id = tex->GetUUID();
			auto it = map.find(id);

			if (it == map.end())
			{
				Ref<GTexture> res = tex;
				TextureDesc tex_desc = res->GetTextureDescription();
				uint32_t tex_size = tex_desc.m_width * tex_desc.m_height * getFmtSize(tex_desc.m_format);
				TRC_INFO("Texture Name: {}, Texture Size: {}", res->GetName(), tex_size);
				total_tex_size += (float)tex_size;
				if (data_size < tex_size)
				{
					if (data)
					{
						delete[] data;// TODO: Use custom allocator
					}
					data = new char[tex_size];
					data_size = tex_size;
				}
				RenderFunc::GetTextureData(res.get(), (void*&)data);
				AssetHeader ast_h;
				ast_h.offset = stream.GetPosition();

				DataStream* data_stream = &stream;
				data_stream->Write(res->GetName());

				stream.Write<uint32_t>(tex_desc.m_width);
				stream.Write<uint32_t>(tex_desc.m_height);
				stream.Write<uint32_t>(tex_desc.m_mipLevels);


				stream.Write<Format>(tex_desc.m_format);

				stream.Write<BindFlag>(tex_desc.m_flag);

				stream.Write<UsageFlag>(tex_desc.m_usage);

				stream.Write<uint32_t>(tex_desc.m_channels);
				stream.Write<uint32_t>(tex_desc.m_numLayers);

				stream.Write<ImageType>(tex_desc.m_image_type);

				stream.Write<AddressMode>(tex_desc.m_addressModeU);
				stream.Write<AddressMode>(tex_desc.m_addressModeV);
				stream.Write<AddressMode>(tex_desc.m_addressModeW);

				stream.Write<FilterMode>(tex_desc.m_minFilterMode);
				stream.Write<FilterMode>(tex_desc.m_magFilterMode);

				stream.Write<AttachmentType>(tex_desc.m_attachmentType);


				stream.Write(data, tex_size);
				ast_h.data_size = stream.GetPosition() - ast_h.offset;
				map.emplace(std::make_pair(id, ast_h));
			}
		};

		auto img_lambda = [&](Entity entity) -> bool {
			ImageComponent& img = entity.GetComponent<ImageComponent>();
			if (!img.image)
			{
				return false;
			}
			
			serialize_tex(img.image);

			return false;
		};

		auto model_lambda = [&](Entity entity) -> bool {
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<MaterialInstance> res = renderer._material;
			for (auto& m_data : res->GetMaterialData())
			{
				if (m_data.second.type == ShaderData::CUSTOM_DATA_TEXTURE)
				{
					Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(m_data.second.internal_data);
					serialize_tex(tex);

				}
			}

			return false;
		};

		auto skinned_model_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<MaterialInstance> res = renderer._material;
			for (auto& m_data : res->GetMaterialData())
			{
				if (m_data.second.type == ShaderData::CUSTOM_DATA_TEXTURE)
				{
					Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(m_data.second.internal_data);
					serialize_tex(tex);

				}
			}

			return false;
		};

		scene->IterateComponent<ImageComponent>(img_lambda);
		scene->IterateComponent<ModelRendererComponent>(model_lambda);
		scene->IterateComponent<SkinnedModelRenderer>(skinned_model_lambda);

		if (data)
		{
			delete[] data;// TODO: Use custom allocator
		}

		float m_b = ((float)MB);
		float t_size = total_tex_size / m_b;
		TRC_INFO("Total Texture Size: {}MB", t_size);

		return true;
	}

	bool SceneSerializer::SerializeAnimationClips(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		auto anim_lambda = [&](Entity entity) -> bool
		{
			AnimationComponent& anim = entity.GetComponent<AnimationComponent>();
			if (!anim.animation)
			{
				return false;
			}
			UUID id = GetUUIDFromName(anim.animation->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			AnimationsSerializer::SerializeAnimationClip(anim.animation, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<AnimationComponent>(anim_lambda);

		std::function<void(void*)> anim_clip_callback = [&](void* location)
		{
			Ref<AnimationClip>& clip = *(Ref<AnimationClip>*)location;
			UUID id = GetUUIDFromName(clip->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			AnimationsSerializer::SerializeAnimationClip(clip, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));
		};

		auto graph_lambda = [&](Entity entity) -> bool
		{
			AnimationGraphController& anim = entity.GetComponent<AnimationGraphController>();
			Ref<Animation::Graph> graph = anim.graph.GetGraph();
			if (!graph)
			{
				return false;
			}
			constexpr uint64_t anim_clip_type_id = Reflection::TypeID<Ref<AnimationClip>>();
			Reflection::CustomMemberCallback(*graph.get(), anim_clip_type_id, anim_clip_callback);

			return false;
		};

		scene->IterateComponent<AnimationGraphController>(graph_lambda);

		auto sequence_lambda = [&](Entity entity) -> bool
		{
			SequencePlayer& player = entity.GetComponent<SequencePlayer>();
			Ref<Animation::Sequence> sequence = player.sequence.GetSequence();
			if (!sequence)
			{
				return false;
			}
			constexpr uint64_t anim_clip_type_id = Reflection::TypeID<Ref<AnimationClip>>();
			Reflection::CustomMemberCallback(*sequence.get(), anim_clip_type_id, anim_clip_callback);

			return false;
		};

		scene->IterateComponent<SequencePlayer>(sequence_lambda);

		return true;
	}

	bool SceneSerializer::SerializeAnimationGraphs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		auto graph_lambda = [&](Entity entity) -> bool
		{
			AnimationGraphController& anim = entity.GetComponent<AnimationGraphController>();
			Ref<Animation::Graph> graph = anim.graph.GetGraph();
			if (!graph)
			{
				return false;
			}
			
			UUID id = GetUUIDFromName(graph->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();

			AnimationsSerializer::SerializeAnimGraph(graph, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

			return false;

		};

		scene->IterateComponent<AnimationGraphController>(graph_lambda);

		return true;
	}

	/*
	* Font
	*  '-> file_size
	*  '-> file_data
	*/
	bool SceneSerializer::SerializeFonts(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		
		auto fnt_lambda = [&](Entity entity) -> bool
		{
			TextComponent& txt = entity.GetComponent<TextComponent>();
			if (!txt.font)
			{
				return false;
			}
			UUID id = GetUUIDFromName(txt.font->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}

			std::filesystem::path p = GetPathFromUUID(id);
			FileHandle file_handle;
			if (FileSystem::open_file(p.string(), (FileMode)(FileMode::READ | FileMode::BINARY), file_handle))
			{
				DataStream* data_stream = &stream;
				AssetHeader ast_h;
				ast_h.offset = stream.GetPosition();
				data_stream->Write<std::string>(txt.font->GetName());
				uint32_t file_size = 0;
				FileSystem::read_all_bytes(file_handle, nullptr, file_size);
				stream.Write<uint32_t>(file_size);
				char* font_data = new char[file_size];// TODO: Use custom allocator
				FileSystem::read_all_bytes(file_handle, font_data, file_size);
				stream.Write(font_data, file_size);
				delete[] font_data;// TODO: Use custom allocator
				ast_h.data_size = stream.GetPosition() - ast_h.offset;
				FileSystem::close_file(file_handle);

				map.emplace(std::make_pair(id, ast_h));
			}
			else
			{
				TRC_ERROR("Failed to open font file, path -> {}", p.string());
			}
			
			return false;
		};

		scene->IterateComponent<TextComponent>(fnt_lambda);

		return true;
	}

	/*
	* Model
	*  '-> vertex_count
	*  '-> vertices
	*  '-> index_count
	*  '-> indicies
	*/
	bool SceneSerializer::SerializeModels(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		
		auto mdl_lambda = [&](Entity entity) -> bool
		{
			ModelComponent& comp = entity.GetComponent<ModelComponent>();
			if (comp._model)
			{
				return false;
			}
			UUID id = GetUUIDFromName(comp._model->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}
			Ref<Model> res = comp._model;
			AssetHeader ast_h;
			ast_h.offset = stream.GetPosition();
			std::string model_name = res->GetName();
			Reflection::Serialize(model_name, &stream, nullptr, Reflection::SerializationFormat::BINARY);
			Reflection::Serialize(*res.get(), &stream, nullptr, Reflection::SerializationFormat::BINARY);
			ast_h.data_size = stream.GetPosition() - ast_h.offset;

			map.emplace(std::make_pair(id, ast_h));

			return false;
		};

		scene->IterateComponent<ModelComponent>(mdl_lambda);


		return true;
	}

	bool SceneSerializer::SerializeMaterials(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		

		auto mat_lambda = [&](Entity entity) -> bool
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return false;
			}
			UUID id = renderer._material->GetUUID();
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			MaterialSerializer::Serialize(renderer._material, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return false;
			}
			UUID id = renderer._material->GetUUID();
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			MaterialSerializer::Serialize(renderer._material, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<SkinnedModelRenderer>(skin_mat_lambda);

		return true;
	}

	bool SceneSerializer::SerializeShaderGraphs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		auto mat_lambda = [&](Entity entity) -> bool
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return false;
			}

			if (!pipeline->GetShaderGraph())
			{
				return false;
			}

			UUID id = pipeline->GetShaderGraph()->GetUUID();
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}

			Ref<ShaderGraph> graph = GenericAssetManager::get_instance()->Get<ShaderGraph>(id);


			AssetHeader header = {};
			header.offset = stream.GetPosition();

			GenericSerializer::Serialize<ShaderGraph>(graph, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return false;
			}

			if (!pipeline->GetShaderGraph())
			{
				return false;
			}

			UUID id = pipeline->GetShaderGraph()->GetUUID();
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}

			Ref<ShaderGraph> graph = GenericAssetManager::get_instance()->Get<ShaderGraph>(id);


			AssetHeader header = {};
			header.offset = stream.GetPosition();

			GenericSerializer::Serialize<ShaderGraph>(graph, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<SkinnedModelRenderer>(skin_mat_lambda);

		return true;
	}

	bool SceneSerializer::SerializePipelines(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		
		auto mat_lambda = [&](Entity entity) -> bool
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return false;
			}
			UUID id = GetUUIDFromName(pipeline->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();

			PipelineSerializer::Serialize(pipeline, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return false;
			}
			UUID id = GetUUIDFromName(pipeline->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();

			PipelineSerializer::Serialize(pipeline, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));
			
			return false;
		};

		scene->IterateComponent<SkinnedModelRenderer>(skin_mat_lambda);

		return true;
	}

	bool SceneSerializer::SerializeShaders(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		auto shad_lambda = [&](GShader* shader)
		{
			if (shader == nullptr)
			{
				return;
			}

			Ref<GShader> shader_ref = GenericAssetManager::get_instance()->Get<GShader>(shader->GetName());

			UUID id = GetUUIDFromName(shader_ref->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();

			PipelineSerializer::SerializeShader(shader_ref, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));

		};

		auto mat_lambda = [&](Entity entity) -> bool
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return false;
			}
			PipelineStateDesc ds = pipeline->GetDesc();
			shad_lambda(ds.vertex_shader);
			shad_lambda(ds.pixel_shader);

			return false;
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return false;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return false;
			}
			PipelineStateDesc ds = pipeline->GetDesc();
			shad_lambda(ds.vertex_shader);
			shad_lambda(ds.pixel_shader);

			return false;
		};

		scene->IterateComponent<SkinnedModelRenderer>(skin_mat_lambda);

		return true;
	}

	bool SceneSerializer::SerializePrefabs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		auto serialize_prefab = [&](Ref<Prefab> prefab)
		{

			UUID id = prefab->GetUUID();
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();

			SerializePrefab(prefab, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));
		};

		auto prefab_lambda = [&](Entity entity) -> bool
		{
			PrefabComponent& comp = entity.GetComponent<PrefabComponent>();
			Ref<Prefab> prefab = comp.handle;
			if (!prefab)
			{
				return false;
			}

			serialize_prefab(prefab);
			return false;
		};

		scene->IterateComponent<PrefabComponent>(prefab_lambda);

		std::unordered_map<Script*, FieldManager>& field_instances = scene->m_scriptRegistry.GetFieldInstances();

		for (auto [script, field] : field_instances)
		{
			for (auto [id, field_instance] : field)
			{
				for (auto [name, data] : field_instance.GetFields())
				{
					switch (data.type)
					{
					case ScriptFieldType::Prefab:
					{
						UUID id = 0;
						memcpy(&id, data.data, sizeof(UUID));
						if (id != 0)
						{
							Ref<Prefab> asset = GenericAssetManager::get_instance()->Get<Prefab>(id);
							serialize_prefab(asset);
						}
						break;
					}
					}
				}
			}
		}

		Scene* prefab_scene = PrefabManager::get_instance()->GetScene();

		if (scene.get() == prefab_scene)
		{
			return true;
		}

		Ref<Scene> scene_handle(prefab_scene, [](Resource*) {});
		SceneSerializer::SerializeTextures(stream, map, scene_handle);
		SceneSerializer::SerializeAnimationClips(stream, map, scene_handle);
		SceneSerializer::SerializeFonts(stream, map, scene_handle);
		SceneSerializer::SerializeMaterials(stream, map, scene_handle);
		SceneSerializer::SerializeModels(stream, map, scene_handle);
		SceneSerializer::SerializePipelines(stream, map, scene_handle);
		SceneSerializer::SerializeShaders(stream, map, scene_handle);
		SceneSerializer::SerializePrefabs(stream, map, scene_handle);
		SceneSerializer::SerializeAnimationGraphs(stream, map, scene_handle);
		SceneSerializer::SerializeSkeletons(stream, map, scene_handle);
		SceneSerializer::SerializeSequences(stream, map, scene_handle);
		SceneSerializer::SerializeSkinnedModels(stream, map, scene_handle);
		SceneSerializer::SerializeShaderGraphs(stream, map, scene_handle);

		return true;
	}

	bool SceneSerializer::SerializeSkeletons(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		std::function<void(void*)> skeleton_callback = [&](void* location)
		{
			Ref<Animation::Skeleton>& skeleton = *(Ref<Animation::Skeleton>*)location;
			UUID id = GetUUIDFromName(skeleton->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			AnimationsSerializer::SerializeSkeleton(skeleton, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));
		};

		auto graph_lambda = [&](Entity entity) -> bool
		{
			AnimationGraphController& anim = entity.GetComponent<AnimationGraphController>();
			Ref<Animation::Graph> graph = anim.graph.GetGraph();
			if (!graph)
			{
				return false;
			}
			uint64_t skeleton_type_id = Reflection::TypeID<Ref<Animation::Skeleton>>();
			Reflection::CustomMemberCallback(*graph.get(), skeleton_type_id, skeleton_callback);

			return false;
		};

		scene->IterateComponent<AnimationGraphController>(graph_lambda);

		auto skinned_model_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			uint64_t skeleton_type_id = Reflection::TypeID<Ref<Animation::Skeleton>>();
			Reflection::CustomMemberCallback(renderer, skeleton_type_id, skeleton_callback);

			return false;

		};

		scene->IterateComponent<SkinnedModelRenderer>(skinned_model_lambda);


		return true;
	}

	bool SceneSerializer::SerializeSequences(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		auto sequence_lambda = [&](Entity entity) -> bool
		{
			SequencePlayer& anim = entity.GetComponent<SequencePlayer>();
			Ref<Animation::Sequence> sequence = anim.sequence.GetSequence();
			if (!sequence)
			{
				return false;
			}

			UUID id = GetUUIDFromName(sequence->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();

			AnimationsSerializer::SerializeSequence(sequence, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<SequencePlayer>(sequence_lambda);

		return true;
	}

	bool SceneSerializer::SerializeSkinnedModels(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		auto skinned_model_lambda = [&](Entity entity) -> bool
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._model)
			{
				return false;
			}
			UUID id = GetUUIDFromName(renderer._model->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return false;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();
			std::string model_name = renderer._model->GetName();
			Reflection::Serialize(model_name , &stream, nullptr, Reflection::SerializationFormat::BINARY);
			Reflection::Serialize(*renderer._model.get(), &stream, nullptr, Reflection::SerializationFormat::BINARY);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

			return false;
		};

		scene->IterateComponent<SkinnedModelRenderer>(skinned_model_lambda);

		return true;
	}

}