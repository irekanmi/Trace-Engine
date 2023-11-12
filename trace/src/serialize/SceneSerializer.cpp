#include "pch.h"

#include "SceneSerializer.h"
#include "scene/Entity.h"
#include "scene/Componets.h"
#include "core/FileSystem.h"
#include "scene/SceneManager.h"
#include "resource/MeshManager.h"
#include "resource/FontManager.h"
#include "resource/ModelManager.h"
#include "resource/MaterialManager.h"
#include "serialize/MaterialSerializer.h"
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
				if(Txt.font)
					emit << YAML::Key << "Font file_id" << YAML::Value << GetUUIDFromName(Txt.font->GetName());

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
		else res = MaterialSerializer::Deserialize(p.string());
		if (res) model_renderer._material = res;

		}},
		{"TextComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["TextComponent"];
		TextComponent& Txt = entity.AddComponent<TextComponent>();
		Txt.text = comp["Text Data"].as<std::string>();
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

		}}
	};

	

	static bool SerializeEntites(Ref<Scene> scene, Entity entity, YAML::Emitter& emit)
	{
		IDComponent& uuid = entity.GetComponent<IDComponent>();

		emit << YAML::BeginMap;
		emit << YAML::Key << "UUID" << YAML::Value << uuid._id;

		for (uint32_t i = 0; i < ARRAYSIZE(_serialize_components); i++)
		{
			_serialize_components[i](entity, emit);
		}

		emit << YAML::EndMap;
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

		for (auto& [entity] : scene->m_registry.storage<entt::entity>().each())
		{
			Entity en(entity, scene.get());
			SerializeEntites(scene, en, emit);
		}

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
		FileSystem::read_all_lines(in_handle, file_data);
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
				Entity obj = scene->CreateEntity_UUID(uuid, "");
				for (auto& i : _deserialize_components)
				{
					if (entity[i.first]) i.second(obj, entity);
				}
			}
		}
		
		return scene;
	}

}