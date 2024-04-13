#include "pch.h"

#include "SceneSerializer.h"
#include "scene/Entity.h"
#include "scene/Components.h"
#include "core/FileSystem.h"
#include "scene/SceneManager.h"
#include "resource/MeshManager.h"
#include "resource/FontManager.h"
#include "resource/ModelManager.h"
#include "resource/MaterialManager.h"
#include "resource/AnimationsManager.h"
#include "resource/TextureManager.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "serialize/PipelineSerializer.h"
#include "scripting/ScriptEngine.h"
#include "backends/Renderutils.h"
#include "resource/ShaderManager.h"

#include <functional>
#include <unordered_map>

#include "yaml_util.h"

namespace trace {

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	static std::function<void(Entity entity, YAML::Emitter& emit)> _serialize_components[] = {
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<TagComponent>())
			{
			TagComponent& tag = entity.GetComponent<TagComponent>();
			emit << YAML::Key << "TagComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Tag" << YAML::Value << tag._tag;

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<HierachyComponent>())
			{
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			emit << YAML::Key << "Parent" << YAML::Value << hi.parent;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<TransformComponent>())
			{
			TransformComponent& trans = entity.GetComponent<TransformComponent>();
			emit << YAML::Key << "TransformComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Position" << YAML::Value << trans._transform.GetPosition();
			emit << YAML::Key << "Rotation" << YAML::Value << trans._transform.GetRotation();
			emit << YAML::Key << "Scale" << YAML::Value << trans._transform.GetScale();

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<LightComponent>())
			{
			LightComponent& light = entity.GetComponent<LightComponent>();
			emit << YAML::Key << "LightComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Color" << YAML::Value << light._light.color;
			emit << YAML::Key << "Direction" << YAML::Value << light._light.direction;
			emit << YAML::Key << "Position" << YAML::Value << light._light.position;
			emit << YAML::Key << "Params1" << YAML::Value << light._light.params1;
			emit << YAML::Key << "Params2" << YAML::Value << light._light.params2;
			emit << YAML::Key << "Light Type" << YAML::Value << (int)light.light_type;

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<CameraComponent>())
			{
			CameraComponent& cam = entity.GetComponent<CameraComponent>();
			emit << YAML::Key << "CameraComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Is Main" << YAML::Value << cam.is_main;
			emit << YAML::Key << "Camera Type" << YAML::Value << (int)cam._camera.GetCameraType();
			emit << YAML::Key << "Position" << YAML::Value << cam._camera.GetPosition();
			emit << YAML::Key << "Look Direction" << YAML::Value << cam._camera.GetLookDir();
			emit << YAML::Key << "Up Direction" << YAML::Value << cam._camera.GetUpDir();
			emit << YAML::Key << "Near" << YAML::Value << cam._camera.GetNear();
			emit << YAML::Key << "Far" << YAML::Value << cam._camera.GetFar();
			emit << YAML::Key << "Fov" << YAML::Value << cam._camera.GetFov();
			emit << YAML::Key << "Aspect Ratio" << YAML::Value << cam._camera.GetAspectRatio();
			emit << YAML::Key << "Orthographic Size" << YAML::Value << cam._camera.GetOrthographicSize();

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<MeshComponent>())
			{
			MeshComponent& mesh = entity.GetComponent<MeshComponent>();
			emit << YAML::Key << "MeshComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "file id" << YAML::Value << GetUUIDFromName(mesh._mesh->m_path.filename().string());

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<ModelComponent>())
			{
			ModelComponent& model = entity.GetComponent<ModelComponent>();
			if (!model._model) return;
			emit << YAML::Key << "ModelComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Name" << model._model->GetName();
			std::string filename = model._model->m_path.parent_path().filename().string();
			if (filename.empty()) filename = model._model->m_path.filename().string();
			emit << YAML::Key << "file id" << YAML::Value << GetUUIDFromName(filename);

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<ModelRendererComponent>())
			{
				ModelRendererComponent& model_renderer = entity.GetComponent<ModelRendererComponent>();
				emit << YAML::Key << "ModelRendererComponent" << YAML::Value;
				emit << YAML::BeginMap;
				emit << YAML::Key << "file id" << YAML::Value << GetUUIDFromName(model_renderer._material->GetName());

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<TextComponent>())
			{
				TextComponent& Txt = entity.GetComponent<TextComponent>();
				emit << YAML::Key << "TextComponent" << YAML::Value;
				emit << YAML::BeginMap;
				emit << YAML::Key << "Text Data" << YAML::Value << Txt.text;
				emit << YAML::Key << "Color" << YAML::Value << Txt.color;
				emit << YAML::Key << "Intensity" << YAML::Value << Txt.intensity;
				if(Txt.font)
					emit << YAML::Key << "Font file_id" << YAML::Value << GetUUIDFromName(Txt.font->GetName());

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<RigidBodyComponent>())
			{
				RigidBodyComponent& body = entity.GetComponent<RigidBodyComponent>();
				emit << YAML::Key << "RigidBodyComponent" << YAML::Value;
				emit << YAML::BeginMap;
				emit << YAML::Key << "Type" << YAML::Value << (int)body.body.GetType();
				emit << YAML::Key << "Density" << YAML::Value << body.body.density;
				emit << YAML::Key << "Mass" << YAML::Value << body.body.mass;

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<BoxColliderComponent>())
			{
				BoxColliderComponent& box = entity.GetComponent<BoxColliderComponent>();
				emit << YAML::Key << "BoxCoillderComponent" << YAML::Value;
				emit << YAML::BeginMap;
				emit << YAML::Key << "Is Trigger" << YAML::Value << box.is_trigger;
				emit << YAML::Key << "Extents" << YAML::Value << box.shape.box.half_extents;
				emit << YAML::Key << "Offset" << YAML::Value << box.shape.offset;

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<SphereColliderComponent>())
			{
				SphereColliderComponent& sc = entity.GetComponent<SphereColliderComponent>();
				emit << YAML::Key << "SphereColliderComponent" << YAML::Value;
				emit << YAML::BeginMap;
				emit << YAML::Key << "Is Trigger" << YAML::Value << sc.is_trigger;
				emit << YAML::Key << "Radius" << YAML::Value << sc.shape.sphere.radius;
				emit << YAML::Key << "Offset" << YAML::Value << sc.shape.offset;

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<AnimationComponent>())
			{
				AnimationComponent& ac = entity.GetComponent<AnimationComponent>();
				emit << YAML::Key << "AnimationComponent" << YAML::Value;
				emit << YAML::BeginMap;
				if(ac.anim_graph) emit << YAML::Key << "Anim Graph" << YAML::Value << GetUUIDFromName(ac.anim_graph->GetName());
				emit << YAML::Key << "Play On Start" << YAML::Value << ac.play_on_start;

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<ImageComponent>())
			{
				ImageComponent& img = entity.GetComponent<ImageComponent>();
				emit << YAML::Key << "ImageComponent" << YAML::Value;
				emit << YAML::BeginMap;
				if (img.image) emit << YAML::Key << "Image" << YAML::Value << GetUUIDFromName(img.image->GetName());

				emit << YAML::EndMap;
			}
		}
	};

	static std::unordered_map<std::string, std::function<void(Entity entity, YAML::detail::iterator_value& value)>> _deserialize_components = {
		{"TagComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["TagComponent"];
			auto data = comp["Tag"];
			TagComponent& tag = entity.GetComponent<TagComponent>();
			tag._tag = data.as<std::string>();
		}},
		{"TransformComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["TransformComponent"];
			TransformComponent& trans = entity.GetComponent<TransformComponent>();
			trans._transform.SetPosition(comp["Position"].as<glm::vec3>());
			trans._transform.SetRotation(comp["Rotation"].as<glm::quat>());
			trans._transform.SetScale(comp["Scale"].as<glm::vec3>());
		}},
		{"LightComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["LightComponent"];
			LightComponent& lit = entity.AddComponent<LightComponent>();
			lit._light.color = comp["Color"].as<glm::vec4>();
			lit._light.direction = comp["Direction"].as<glm::vec4>();
			lit._light.position = comp["Position"].as<glm::vec4>();
			lit._light.params1 = comp["Params1"].as<glm::vec4>();
			lit._light.params2 = comp["Params2"].as<glm::vec4>();
			lit.light_type = (LightType)comp["Light Type"].as<int>();
		}},
		{"CameraComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["CameraComponent"];
			CameraComponent& cam = entity.AddComponent<CameraComponent>();
			cam.is_main = comp["Is Main"].as<bool>();
			cam._camera.SetCameraType((CameraType)comp["Camera Type"].as<int>());
			cam._camera.SetPosition(comp["Position"].as<glm::vec3>());
			cam._camera.SetLookDir(comp["Look Direction"].as<glm::vec3>());
			cam._camera.SetUpDir(comp["Up Direction"].as<glm::vec3>());
			cam._camera.SetNear(comp["Near"].as<float>());
			cam._camera.SetFar(comp["Far"].as<float>());
			cam._camera.SetFov(comp["Fov"].as<float>());
			cam._camera.SetAspectRatio(comp["Aspect Ratio"].as<float>());
			cam._camera.SetOrthographicSize(comp["Orthographic Size"].as<float>());
		}},
		{"MeshComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["MeshComponent"];
			MeshComponent& mesh = entity.AddComponent<MeshComponent>();
			mesh._mesh = MeshManager::get_instance()->GetDefault("Cube");
			Ref<Mesh> res;
			UUID id = comp["file id"].as<uint64_t>();
			std::filesystem::path p = GetPathFromUUID(id);
			res = MeshManager::get_instance()->GetMesh(p.filename().string());
			if (res) {}
			else res = MeshManager::get_instance()->LoadMesh_(p.string());
			if (res) mesh._mesh = res;

		}},
		{"ModelComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["ModelComponent"];
			ModelComponent& model = entity.AddComponent<ModelComponent>();
			model._model = ModelManager::get_instance()->GetModel(comp["Name"].as<std::string>());
			UUID id = comp["file id"].as<uint64_t>();
			std::filesystem::path p = GetPathFromUUID(id);
			Ref<Mesh> res;
			res = MeshManager::get_instance()->GetMesh(p.filename().string());
			if (res) {}
			else res = MeshManager::get_instance()->LoadMeshOnly_(p.string());
			if (res)
			{
				model._model = ModelManager::get_instance()->GetModel(comp["Name"].as<std::string>());
			}

		}},
		{"ModelRendererComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["ModelRendererComponent"];
		ModelRendererComponent& model_renderer = entity.AddComponent<ModelRendererComponent>();
		Ref<MaterialInstance> res;
		UUID id = comp["file id"].as<uint64_t>();
		std::filesystem::path p = GetPathFromUUID(id);
		res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
		if (res) {}
		else
		{
			res = MaterialSerializer::Deserialize(p.string());
		}
		if (res) model_renderer._material = res;

		}},
		{"TextComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["TextComponent"];
		TextComponent& Txt = entity.AddComponent<TextComponent>();
		if(comp["Text Data"]) Txt.text = comp["Text Data"].as<std::string>();
		if(comp["Color"]) Txt.color = comp["Color"].as<glm::vec3>();
		if(comp["Intensity"]) Txt.intensity = comp["Intensity"].as<float>();
		if (comp["Font file_id"])
		{
			Ref<Font> res;
			UUID id = comp["Font file_id"].as<uint64_t>();
			std::filesystem::path p = GetPathFromUUID(id);
			res = FontManager::get_instance()->GetFont(p.filename().string());
			if (res) {}
			else res = FontManager::get_instance()->LoadFont_(p.string());
			if (res) Txt.font = res;
		}

		}},
		{ "RigidBodyComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["RigidBodyComponent"];
		RigidBodyComponent& body = entity.AddComponent<RigidBodyComponent>();
		body.body.SetType((RigidBody::Type)comp["Type"].as<int>());
		body.body.density = comp["Density"].as<float>();
		body.body.mass = comp["Mass"].as<float>();


		}},
		{ "BoxCoillderComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["BoxCoillderComponent"];
		BoxColliderComponent& box = entity.AddComponent<BoxColliderComponent>();
		box.is_trigger = comp["Is Trigger"].as<bool>();
		box.shape.SetBox(comp["Extents"].as<glm::vec3>());
		box.shape.offset = comp["Offset"].as<glm::vec3>();

		}},
		{ "SphereColliderComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["SphereColliderComponent"];
		SphereColliderComponent& sc = entity.AddComponent<SphereColliderComponent>();
		sc.is_trigger = comp["Is Trigger"].as<bool>();
		sc.shape.SetSphere(comp["Radius"].as<float>());
		sc.shape.offset = comp["Offset"].as<glm::vec3>();

		} },
		{ "AnimationComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["AnimationComponent"];
		AnimationComponent& ac = entity.AddComponent<AnimationComponent>();
		ac.play_on_start = comp["Play On Start"].as<bool>();
		if (comp["Anim Graph"])
		{
			Ref<AnimationGraph> res;
			UUID id = comp["Anim Graph"].as<uint64_t>();
			std::filesystem::path p = GetPathFromUUID(id);
			res = AnimationsManager::get_instance()->GetGraph(p.filename().string());
			if (res) {}
			else res = AnimationsSerializer::DeserializeAnimationGraph(p.string());
			if (res) ac.anim_graph = res;
		}

		} },
		{ "ImageComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["ImageComponent"];
		ImageComponent& img = entity.AddComponent<ImageComponent>();
		if (comp["Image"])
		{
			Ref<GTexture> res;
			UUID id = comp["Image"].as<uint64_t>();
			std::filesystem::path p = GetPathFromUUID(id);
			res = TextureManager::get_instance()->GetTexture(p.filename().string());
			if (res) {}
			else res = TextureManager::get_instance()->LoadTexture_(p.string());
			if (res) img.image = res;
		}

		} }
	};

	

	static bool SerializeEntites(Ref<Scene> scene, Entity entity, YAML::Emitter& emit)
	{
		IDComponent& uuid = entity.GetComponent<IDComponent>();

		emit << YAML::Key << "UUID" << YAML::Value << uuid._id;

		for (uint32_t i = 0; i < ARRAYSIZE(_serialize_components); i++)
		{
			_serialize_components[i](entity, emit);
		}

		return true;
	}

	bool SceneSerializer::Serialize(Ref<Scene> scene, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Scene Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Scene Name" << YAML::Value << scene->GetName();
		emit << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto process_hierachy = [&](Entity entity, UUID, Scene*)
		{
			Entity en(entity, scene.get());
			emit << YAML::BeginMap;
			SerializeEntites(scene, en, emit);


			//Scripts ----------------------------------------
			emit << YAML::Key << "Scripts" << YAML::Value;
			emit << YAML::BeginSeq;

			ScriptRegistry& script_registry = scene->m_scriptRegistry;

			script_registry.Iterate(en.GetID(), [&](UUID uuid, Script* script, ScriptInstance* instance)
				{
					//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
					auto& fields_instances = script_registry.GetFieldInstances();
					auto& field_manager = fields_instances[script];
					auto field_it = field_manager.find(uuid);
					bool has_fields = !(field_it == field_manager.end());
					if (!has_fields)
					{
						TRC_WARN("entity id:{} does not have a field instance with script:{}", (uint64_t)uuid, script->script_name);
					}
					emit << YAML::BeginMap;

					emit << YAML::Key << "Script Name" << YAML::Value << script->script_name;
					emit << YAML::Key << "Script Values" << YAML::Value << YAML::BeginSeq; // Script values
					if (has_fields)
					{
						ScriptFieldInstance& ins = field_manager[uuid];
						for (auto& [name, field] : ins.m_fields)
						{
							emit << YAML::BeginMap;

							emit << YAML::Key << "Name" << YAML::Value << name;
							//emit << YAML::Key << "Type" << YAML::Value << (int)field.type;
							switch (field.type)
							{
							case ScriptFieldType::String:
							{
								break;
							}
							case ScriptFieldType::Bool:
							{
								bool data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Byte:
							{
								char data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Double:
							{
								double data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Char:
							{
								char data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Float:
							{
								float data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Int16:
							{
								int16_t data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Int32:
							{
								int32_t data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Int64:
							{
								int64_t data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::UInt16:
							{
								uint16_t data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::UInt32:
							{
								uint32_t data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::UInt64:
							{
								uint64_t data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Vec2:
							{
								glm::vec2 data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Vec3:
							{
								glm::vec3 data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}
							case ScriptFieldType::Vec4:
							{
								glm::vec4 data;
								ins.GetValue(name, data);
								emit << YAML::Key << "Value" << YAML::Value << data;
								break;
							}

							}

							emit << YAML::EndMap;
						}
					}
					emit << YAML::EndSeq; // Script values

					emit << YAML::EndMap;
				});

			emit << YAML::EndSeq;
			// --------------------------------------------------

			emit << YAML::EndMap;
		};

		scene->ProcessEntitiesByHierachy(process_hierachy);

		emit << YAML::EndSeq;

		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file(file_path, FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}

		return true;
	}

	Ref<Scene> SceneSerializer::Deserialize(const std::string& file_path)
	{
		FileHandle in_handle;
		if (!FileSystem::open_file(file_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return Ref<Scene>();
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data); // opening file
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file {}", file_path);
			return Ref<Scene>();
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string scene_version = data["Scene Version"].as<std::string>(); // TODO: To be used later
		std::string scene_name = data["Scene Name"].as<std::string>();

		Ref<Scene> scene = SceneManager::get_instance()->GetScene(scene_name);
		if (scene)
		{
			TRC_WARN("{} has already been loaded", scene_name);
			return scene;
		}
		scene = SceneManager::get_instance()->CreateScene(scene_name);
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				UUID uuid = entity["UUID"].as<uint64_t>();
				UUID parent = 0;
				if (entity["Parent"]) parent = entity["Parent"].as<uint64_t>();
				Entity obj = scene->CreateEntity_UUID(uuid, "", parent);
				for (auto& i : _deserialize_components)
				{
					if (entity[i.first]) i.second(obj, entity);
				}
				YAML::Node scripts = entity["Scripts"];
				if (!scripts) continue;
				std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
				
				for (auto script : scripts)
				{
					std::string script_name = script["Script Name"].as<std::string>();
					auto it = g_Scripts.find(script_name);
					if (it == g_Scripts.end())
					{
						TRC_WARN("Script:{} does not exist in the assembly", script_name);
						continue;
					}
					obj.AddScript(script_name);

					ScriptRegistry& script_registry = scene->m_scriptRegistry;

					//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
					auto& fields_instances = script_registry.GetFieldInstances();
					auto& field_manager = fields_instances[&it->second];
					auto field_it = field_manager.find(uuid);
					if (field_it == field_manager.end())
					{
						ScriptFieldInstance& field_ins = field_manager[obj.GetID()];
						field_ins.Init(&it->second);
					}
					ScriptFieldInstance& ins = field_manager[obj.GetID()];

					for (auto values : script["Script Values"])
					{
						std::string field_name = values["Name"].as<std::string>();
						auto f_it = ins.m_fields.find(field_name);
						if (f_it == ins.m_fields.end())
						{
							TRC_WARN("Script Field:{} does not exist in the Script Class", field_name);
							continue;
						}
						switch (f_it->second.type)
						{
						case ScriptFieldType::String:
						{
							break;
						}
						case ScriptFieldType::Bool:
						{
							bool data = values["Value"].as<bool>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Byte:
						{
							char data = values["Value"].as<char>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Double:
						{
							double data = values["Value"].as<double>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Char:
						{
							char data = values["Value"].as<char>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Float:
						{
							float data = values["Value"].as<float>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Int16:
						{
							int16_t data = values["Value"].as<int16_t>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Int32:
						{
							int32_t data = values["Value"].as<int32_t>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Int64:
						{
							int64_t data = values["Value"].as<int64_t>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::UInt16:
						{
							uint16_t data = values["Value"].as<uint16_t>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::UInt32:
						{
							uint32_t data = values["Value"].as<uint32_t>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::UInt64:
						{
							uint64_t data = values["Value"].as<uint64_t>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Vec2:
						{
							glm::vec2 data = values["Value"].as<glm::vec2>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Vec3:
						{
							glm::vec3 data = values["Value"].as<glm::vec3>();
							ins.SetValue(field_name, data);
							break;
						}
						case ScriptFieldType::Vec4:
						{
							glm::vec4 data = values["Value"].as<glm::vec4>();
							ins.SetValue(field_name, data);
							break;
						}
						}
					}

				}
			}
		}
		
		return scene;
	}

	/*
	* Texture
	*  '-> TextureDesc
	*  '-> texture_data
	*/
	bool SceneSerializer::SerializeTextures(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			char* data = nullptr;// TODO: Use custom allocator
			int data_size = 0;
			for (auto entity : entities)
			{
				if (entity["ImageComponent"])
				{
					auto comp = entity["ImageComponent"];
					if (comp["Image"])
					{
						Ref<GTexture> res;
						UUID id = comp["Image"].as<uint64_t>();
						auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
							{
								return i.first == id;
							});

						if (it == map.end())
						{
							std::filesystem::path p = GetPathFromUUID(id);
							res = TextureManager::get_instance()->LoadTexture_(p.string());
							if (res)
							{
								TextureDesc tex_desc = res->GetTextureDescription();
								uint32_t tex_size = tex_desc.m_width * tex_desc.m_height * getFmtSize(tex_desc.m_format);
								if (data_size < tex_size)
								{
									if (data) delete[] data;// TODO: Use custom allocator
									data = new char[tex_size];
									data_size = tex_size;
								}
								RenderFunc::GetTextureData(res.get(),(void*&) data);
								AssetHeader ast_h;
								ast_h.offset = stream.GetPosition();
								stream.Write<TextureDesc>(tex_desc);
								stream.Write(data, tex_size);
								ast_h.data_size = stream.GetPosition() - ast_h.offset;
								map.push_back(std::make_pair(id, ast_h));
							}
							else
							{
								TRC_ERROR("Unable to able to load image, path -> {}", p.string());
							}
						}
					}
				}
				if (entity["ModelRendererComponent"])
				{
					auto comp = entity["ModelRendererComponent"];
					UUID id = comp["file id"].as<uint64_t>();
					auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
						{
							return i.first == id;
						});

					if (it == map.end())
					{

						std::filesystem::path p = GetPathFromUUID(id);
						Ref<MaterialInstance> res;
						res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
						if (res) {}
						else res = MaterialSerializer::Deserialize(p.string());
						if (res)
						{
							for (auto& m_data : res->m_data)
							{
								trace::UniformMetaData& meta_data = res->m_renderPipeline->Scene_uniforms[m_data.second.second];
								if (meta_data.data_type == ShaderData::CUSTOM_DATA_TEXTURE)
								{
									Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(m_data.second.first);
									TextureDesc tex_desc = tex->GetTextureDescription();
									uint32_t tex_size = tex_desc.m_width * tex_desc.m_height * getFmtSize(tex_desc.m_format);
									if (data_size < tex_size)
									{
										if (data) delete[] data;// TODO: Use custom allocator
										data = new char[tex_size];
										data_size = tex_size;
									}
									RenderFunc::GetTextureData(tex.get(), (void*&)data);
									AssetHeader ast_h;
									ast_h.offset = stream.GetPosition();
									stream.Write<TextureDesc>(tex_desc);
									stream.Write(data, tex_size);
									ast_h.data_size = stream.GetPosition() - ast_h.offset;
									map.push_back(std::make_pair(id, ast_h));
								}
							}
						}
						else
						{
							TRC_ERROR("Unable to load material, path -> {}", p.string());
						}
					}
				}
			}
			if(data) delete[] data;// TODO: Use custom allocator
		}

		return true;
	}

	bool SceneSerializer::SerializeAnimationClips(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{

			for (auto entity : entities)
			{
				if (entity["AnimationComponent"])
				{
					auto comp = entity["AnimationComponent"];
					if (comp["Anim Graph"])
					{
						Ref<AnimationGraph> res;
						UUID id = comp["Anim Graph"].as<uint64_t>();

						std::filesystem::path p = GetPathFromUUID(id);
						res = AnimationsManager::get_instance()->GetGraph(p.filename().string());
						if (res) {}
						else res = AnimationsSerializer::DeserializeAnimationGraph(p.string());
						if (res)
						{
							std::vector<AnimationState>& states = res->GetStates();
							for (AnimationState& state : states)
							{
								if (state.GetAnimationClip())
								{
									Ref<AnimationClip> clip = state.GetAnimationClip();
									AnimationsSerializer::SerializeAnimationClip(clip,  stream,  map);
								}
							}
						}
						else
						{
							TRC_ERROR("Unable to load anim_graph, path -> {}", p.string());
						}

						
					}
				}

			}

		}

		return true;
	}

	bool SceneSerializer::SerializeAnimationGraphs(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{

			for (auto entity : entities)
			{
				if (entity["AnimationComponent"])
				{
					auto comp = entity["AnimationComponent"];
					if (comp["Anim Graph"])
					{
						Ref<AnimationGraph> res;
						UUID id = comp["Anim Graph"].as<uint64_t>();
						auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
							{
								return i.first == id;
							});

						if (it == map.end())
						{
							std::filesystem::path p = GetPathFromUUID(id);
							res = AnimationsManager::get_instance()->GetGraph(p.filename().string());
							if (res) {}
							else res = AnimationsSerializer::DeserializeAnimationGraph(p.string());
							if (res)
							{
								AnimationsSerializer::SerializeAnimationGraph(res, stream, map);
							}
							else
							{
								TRC_ERROR("Unable to load anim_graph, path -> {}", p.string());
							}
						}
					}
				}
			}

		}

		return true;
	}

	/*
	* Font
	*  '-> file_size
	*  '-> file_data
	*/
	bool SceneSerializer::SerializeFonts(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (!entities) return false;

		for (auto entity : entities)
		{
			if (entity["TextComponent"])
			{
				auto comp = entity["TextComponent"];
				if (comp["Font file_id"])
				{
					Ref<Font> res;
					UUID id = comp["Font file_id"].as<uint64_t>();
					auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
						{
							return i.first == id;
						});
					if (it == map.end())
					{
						std::filesystem::path p = GetPathFromUUID(id);
						FileHandle file_handle;
						if (FileSystem::open_file(p.string(), (FileMode)(FileMode::READ | FileMode::BINARY), file_handle))
						{
							AssetHeader ast_h;
							ast_h.offset = stream.GetPosition();
							uint32_t file_size = 0;
							FileSystem::read_all_bytes(file_handle, nullptr, file_size);
							stream.Write<uint32_t>(file_size);
							char* font_data = new char[file_size];// TODO: Use custom allocator
							FileSystem::read_all_bytes(file_handle, font_data, file_size);
							stream.Write(font_data, file_size);
							delete[] font_data;// TODO: Use custom allocator
							ast_h.data_size = stream.GetPosition() - ast_h.offset;
							FileSystem::close_file(file_handle);

							map.push_back(std::make_pair(id, ast_h));
						}
						else
						{
							TRC_ERROR("Failed to open font file, path -> {}", p.string());
						}
					}
				}

			}
		}


		return true;
	}

	/*
	* Model
	*  '-> vertex_count
	*  '-> vertices
	*  '-> index_count
	*  '-> indicies
	*/
	bool SceneSerializer::SerializeModels(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				if (entity["ModelComponent"])
				{
					auto comp = entity["ModelComponent"];
					if (comp["file id"])
					{
						UUID id = comp["file id"].as<uint64_t>();
						auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
							{
								return i.first == id;
							});
						if (it == map.end())
						{
							Ref<Model> res = ModelManager::get_instance()->GetModel(comp["Name"].as<std::string>());
							AssetHeader ast_h;
							ast_h.offset = stream.GetPosition();
							std::vector<Vertex>& verticies = res->GetVertices();
							std::vector<uint32_t>& indices = res->GetIndices();
							int vertex_count = verticies.size();
							int index_count = indices.size();
							stream.Write<int>(vertex_count);
							stream.Write(verticies.data(), vertex_count * sizeof(Vertex));
							stream.Write<int>(index_count);
							stream.Write(indices.data(), index_count * sizeof(uint32_t));
							ast_h.data_size = stream.GetPosition() - ast_h.offset;

							map.push_back(std::make_pair(id, ast_h));
						}

					}

				}
			}
		}


		return true;
	}

	bool SceneSerializer::SerializeMaterials(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				if (entity["ModelRendererComponent"])
				{
					auto comp = entity["ModelRendererComponent"];
					if (comp["file id"])
					{
						Ref<MaterialInstance> res;
						UUID id = comp["file id"].as<uint64_t>();
						std::filesystem::path p = GetPathFromUUID(id);
						res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
						if (res) {}
						else
						{
							res = MaterialSerializer::Deserialize(p.string());
						}
						if (res) MaterialSerializer::Serialize(res, stream, map);
						else
						{
							TRC_ERROR("Failed to load material, path -> {}", p.string());
						}
					}

				}
			}
		}


		return true;
	}

	bool SceneSerializer::SerializePipelines(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				if (entity["ModelRendererComponent"])
				{
					auto comp = entity["ModelRendererComponent"];
					if (comp["file id"])
					{
						Ref<MaterialInstance> res;
						UUID id = comp["file id"].as<uint64_t>();
						std::filesystem::path p = GetPathFromUUID(id);
						res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
						if (res) {}
						else
						{
							res = MaterialSerializer::Deserialize(p.string());
						}
						if (res)
						{
							PipelineSerializer::Serialize(res->GetRenderPipline(), stream, map);
						}
						else
						{
							TRC_ERROR("Failed to load material, path -> {}", p.string());
						}
					}

				}
			}
		}


		return true;
	}

	bool SceneSerializer::SerializeShaders(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map, std::string& scn_data)
	{
		YAML::Node data = YAML::Load(scn_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			TRC_ERROR("These file is not a valid scene file, Function -> {}", __FUNCTION__);
			return false;
		}
		YAML::Node entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				if (entity["ModelRendererComponent"])
				{
					auto comp = entity["ModelRendererComponent"];
					if (comp["file id"])
					{
						Ref<MaterialInstance> res;
						UUID id = comp["file id"].as<uint64_t>();
						std::filesystem::path p = GetPathFromUUID(id);
						res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
						if (res) {}
						else
						{
							res = MaterialSerializer::Deserialize(p.string());
						}
						if (res)
						{
							Ref<GPipeline> pipeline = res->GetRenderPipline();
							PipelineStateDesc ds = pipeline->GetDesc();
							Ref<GShader> vert = ShaderManager::get_instance()->GetShader(ds.vertex_shader->GetName());
							Ref<GShader> frag = ShaderManager::get_instance()->GetShader(ds.pixel_shader->GetName());
							PipelineSerializer::SerializeShader(vert, stream, map);
							PipelineSerializer::SerializeShader(frag, stream, map);
						}
						else
						{
							TRC_ERROR("Failed to load material, path -> {}", p.string());
						}
					}

				}
			}
		}


		return true;
	}

}