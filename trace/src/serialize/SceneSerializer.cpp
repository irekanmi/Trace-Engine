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
#include "resource/GenericAssetManager.h"
#include "resource/TextureManager.h"
#include "resource/PrefabManager.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "serialize/PipelineSerializer.h"
#include "scripting/ScriptEngine.h"
#include "backends/Renderutils.h"
#include "resource/ShaderManager.h"
#include "core/Coretypes.h"
#include "external_utils.h"
#include "animation/AnimationPose.h"
#include "animation/AnimationPoseNode.h"
#include "core/Utils.h"
#include "reflection/TypeHash.h"
#include "reflection/SerializeTypes.h"
#include "serialize/SceneSerializeFunctions.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <typeindex>

#include "yaml_util.h"

namespace trace {


	static std::function<void(Entity entity, YAML::Emitter& emit)> _serialize_components[] = {
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<TagComponent>())
			{
			TagComponent& tag = entity.GetComponent<TagComponent>();
			emit << YAML::Key << "TagComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Tag" << YAML::Value << tag.GetTag();

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<HierachyComponent>())
			{
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			emit << YAML::Key << "Parent" << YAML::Value << hi.parent;
			emit << YAML::Key << "Is Enabled" << YAML::Value << hi.is_enabled;
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
			if (filename.empty())
			{
				filename = model._model->m_path.filename().string();
			}
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
				if (model_renderer._material)
				{
					emit << YAML::Key << "file id" << YAML::Value << GetUUIDFromName(model_renderer._material->GetName());
				}
				emit << YAML::Key << "Cast Shadow" << YAML::Value << model_renderer.cast_shadow;

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<SkinnedModelRenderer>())
			{
				SkinnedModelRenderer& model_renderer = entity.GetComponent<SkinnedModelRenderer>();
				emit << YAML::Key << "SkinnedModelRenderer" << YAML::Value;
				emit << YAML::BeginMap;
				if (model_renderer._material)
				{
					emit << YAML::Key << "file id - material" << YAML::Value << GetUUIDFromName(model_renderer._material->GetName());
				}
				if (model_renderer._model)
				{
					emit << YAML::Key << "file id - model" << YAML::Value << GetUUIDFromName(model_renderer._model->GetName());
					emit << YAML::Key << "Model Name" << YAML::Value << model_renderer._model->GetName();
				}
				if (model_renderer.GetSkeleton())
				{
					emit << YAML::Key << "file id - skeleton" << YAML::Value << GetUUIDFromName(model_renderer.GetSkeleton()->GetName());
				}

				emit << YAML::Key << "Cast Shadow" << YAML::Value << model_renderer.cast_shadow;

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
				if (ac.animation)
				{
					emit << YAML::Key << "Animation Clip" << YAML::Value << GetUUIDFromName(ac.animation->GetName());
				}
				emit << YAML::Key << "Play On Start" << YAML::Value << ac.play_on_start;
				emit << YAML::Key << "Loop" << YAML::Value << ac.loop;

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
				if (img.image)
				{
					emit << YAML::Key << "Image" << YAML::Value << GetUUIDFromName(img.image->GetName());
				}
				emit << YAML::Key << "Color" << img.color;

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<PrefabComponent>())
			{
				PrefabComponent& prefab = entity.GetComponent<PrefabComponent>();
				emit << YAML::Key << "PrefabComponent" << YAML::Value;
				emit << YAML::BeginMap;
				if (prefab.handle) emit << YAML::Key << "Prefab" << YAML::Value << GetUUIDFromName(prefab.handle->GetName());

				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<SunLight>())
			{
				SunLight& light = entity.GetComponent<SunLight>();
				emit << YAML::Key << "SunLight" << YAML::Value;
				emit << YAML::BeginMap;
				
				emit << YAML::Key << "Color" << YAML::Value << light.color;
				emit << YAML::Key << "Intensity" << YAML::Value << light.intensity;
				emit << YAML::Key << "Cast Shadows" << YAML::Value << light.cast_shadows;


				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<PointLight>())
			{
				PointLight& light = entity.GetComponent<PointLight>();
				emit << YAML::Key << "PointLight" << YAML::Value;
				emit << YAML::BeginMap;

				emit << YAML::Key << "Color" << YAML::Value << light.color;
				emit << YAML::Key << "Intensity" << YAML::Value << light.intensity;
				emit << YAML::Key << "Radius" << YAML::Value << light.radius;
				emit << YAML::Key << "Cast Shadows" << YAML::Value << light.cast_shadows;


				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<SpotLight>())
			{
				SpotLight& light = entity.GetComponent<SpotLight>();
				emit << YAML::Key << "SpotLight" << YAML::Value;
				emit << YAML::BeginMap;

				emit << YAML::Key << "Color" << YAML::Value << light.color;
				emit << YAML::Key << "Intensity" << YAML::Value << light.intensity;
				emit << YAML::Key << "innerCutOff" << YAML::Value << light.innerCutOff;
				emit << YAML::Key << "outerCutOff" << YAML::Value << light.outerCutOff;
				emit << YAML::Key << "Cast Shadows" << YAML::Value << light.cast_shadows;


				emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<AnimationGraphController>())
			{
			AnimationGraphController& controller = entity.GetComponent<AnimationGraphController>();
			emit << YAML::Key << "AnimationGraphController" << YAML::Value;
			emit << YAML::BeginMap;
			if (controller.graph.GetGraph())
			{
				emit << YAML::Key << "file id" << YAML::Value << GetUUIDFromName(controller.graph.GetGraph()->GetName());
			}
			emit << YAML::Key << "Play On Start" << YAML::Value << controller.play_on_start;

			emit << YAML::EndMap;
			}
		},
		[](Entity entity, YAML::Emitter& emit)
		{
			if (entity.HasComponent<SequencePlayer>())
			{
			SequencePlayer& player = entity.GetComponent<SequencePlayer>();
			emit << YAML::Key << "SequencePlayer" << YAML::Value;
			emit << YAML::BeginMap;
			if (player.sequence.GetSequence())
			{
				emit << YAML::Key << "file id" << YAML::Value << GetUUIDFromName(player.sequence.GetSequence()->GetName());
			}
			emit << YAML::Key << "Play On Start" << YAML::Value << player.play_on_start;

			emit << YAML::EndMap;
			}
		},
	};

	static std::unordered_map<std::string, std::function<void(Entity entity, YAML::detail::iterator_value& value)>> _deserialize_components = {
		{"TagComponent", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["TagComponent"];
			auto data = comp["Tag"];
			TagComponent& tag = entity.GetComponent<TagComponent>();
			tag.SetTag(data.as<std::string>());
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
			UUID id = comp["file id"].as<uint64_t>();
			std::string filename = comp["Name"].as<std::string>();
			if (AppSettings::is_editor)
			{
				model._model = LoadModel(id, filename);
			}
			else
			{
				model._model = ModelManager::get_instance()->LoadModel_Runtime(id);
			}
		}},
		{"ModelRendererComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["ModelRendererComponent"];
		ModelRendererComponent& model_renderer = entity.AddComponent<ModelRendererComponent>();
		Ref<MaterialInstance> res;
		UUID id = 0;
		if (comp["file id"])
		{
			id = comp["file id"].as<uint64_t>();
		}
		if (comp["Cast Shadow"])
		{
			model_renderer.cast_shadow = comp["Cast Shadow"].as<bool>();
		}
		if (id == 0)
		{
			TRC_TRACE("These entity model renderer doesn't have a material, Name: {}", entity.GetComponent<TagComponent>().GetTag());
			return;
		}
		if (AppSettings::is_editor)
		{
		
			std::filesystem::path p = GetPathFromUUID(id);
			res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
			if (!res)
			{
				res = MaterialSerializer::Deserialize(p.string());
			}
			if (res)
			{
				model_renderer._material = res;
			}
		}
		else
		{
			model_renderer._material = MaterialManager::get_instance()->LoadMaterial_Runtime(id);
		}

		}},
		{ "SkinnedModelRenderer", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["SkinnedModelRenderer"];
		SkinnedModelRenderer& model_renderer = entity.AddComponent<SkinnedModelRenderer>();
		Ref<MaterialInstance> res;

		// Material ---------------------------
		UUID material_id = 0;
		if (comp["file id - material"])
		{
			material_id = comp["file id - material"].as<uint64_t>();
		}
		
		if (material_id != 0)
		{

			if (AppSettings::is_editor)
			{

				std::filesystem::path p = GetPathFromUUID(material_id);
				res = MaterialSerializer::Deserialize(p.string());
				model_renderer._material = res;	
			}
			else
			{
				model_renderer._material = MaterialManager::get_instance()->LoadMaterial_Runtime(material_id);
			}
		}
		else
		{
			TRC_TRACE("These entity skinned model renderer doesn't have a material, Name: {}", entity.GetComponent<TagComponent>().GetTag());
		}

		// ----------------------------------


		// Model ------------------

		if (comp["file id - model"])
		{

			UUID model_id = comp["file id - model"].as<uint64_t>();
			std::string model_name = comp["Model Name"].as<std::string>();
			if (model_id != 0 || !model_name.empty())
			{
				if (AppSettings::is_editor)
				{
					model_renderer._model = LoadSkinnedModel(model_id, model_name);
				}
				else
				{
					//model_renderer._model = ModelManager::get_instance()->LoadModel_Runtime(id);
				}
			}
		}

		// --------------------------


		// Skeleton ------------------

		if (comp["file id - skeleton"])
		{

			UUID skeleton_id = comp["file id - skeleton"].as<uint64_t>();
			if (skeleton_id != 0)
			{
				if (AppSettings::is_editor)
				{
					Ref<Animation::Skeleton> skeleton = AnimationsSerializer::DeserializeSkeleton(GetPathFromUUID(skeleton_id).string());
					if (skeleton)
					{
						model_renderer.SetSkeleton(skeleton, entity.GetScene(), entity.GetID());
					}
				}
				else
				{
				}
			}

		}

		// --------------------------


		if (comp["Cast Shadow"])
		{
			model_renderer.cast_shadow = comp["Cast Shadow"].as<bool>();
		}

		

		} },
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
			if (AppSettings::is_editor)
			{
				std::filesystem::path p = GetPathFromUUID(id);
				res = FontManager::get_instance()->GetFont(p.filename().string());
				if (res) {}
				else res = FontManager::get_instance()->LoadFont_(p.string());
				if (res) Txt.font = res;
			}
			else
			{
				Txt.font = FontManager::get_instance()->LoadFont_Runtime(id);
			}
		}

		}},
		{ "RigidBodyComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["RigidBodyComponent"];
		RigidBodyComponent& body = entity.AddComponent<RigidBodyComponent>();
		body.body.SetType((RigidBodyType)comp["Type"].as<int>());
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
		if (comp["Loop"])
		{
			ac.play_on_start = comp["Loop"].as<bool>();

		}
		if (comp["Animation Clip"])
		{
			Ref<AnimationClip> res;
			UUID id = comp["Animation Clip"].as<uint64_t>();
			if (AppSettings::is_editor)
			{
				std::filesystem::path p = GetPathFromUUID(id);
				res = AnimationsSerializer::DeserializeAnimationClip(p.string());
			}
			ac.animation = res;
		}

		} },
		{ "ImageComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["ImageComponent"];
		ImageComponent& img = entity.AddComponent<ImageComponent>();
		if (comp["Image"])
		{
			Ref<GTexture> res;
			UUID id = comp["Image"].as<uint64_t>();
			if (AppSettings::is_editor)
			{
				img.image = LoadTexture(id);
			}
			else
			{
				img.image = TextureManager::get_instance()->LoadTexture_Runtime(id);
			}

			if (comp["Color"])
			{
				img.color = comp["Color"].as<uint32_t>();
			}
		}

		} },
		{ "PrefabComponent", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["PrefabComponent"];
		PrefabComponent& prefab = entity.AddComponent<PrefabComponent>();
		if (comp["Prefab"])
		{
			Ref<Prefab> res;
			UUID id = comp["Prefab"].as<uint64_t>();
			if (AppSettings::is_editor)
			{
				std::filesystem::path p = GetPathFromUUID(id);
				res = PrefabManager::get_instance()->Get(p.filename().string());
				if (res) {}
				else res = SceneSerializer::DeserializePrefab(p.string());
				if (res) prefab.handle = res;
			}
			else
			{
				//TODO: Implement loading prefabs at runtime
				//prefab.handle = PrefabManager::get_instance()->Load_Runtime(id);
			}
		}

		} },
		{ "SunLight", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["SunLight"];
		SunLight& light = entity.AddComponent<SunLight>();
		
		light.color = comp["Color"].as<glm::vec3>();
		light.intensity = comp["Intensity"].as<float>();
		light.cast_shadows = comp["Cast Shadows"].as<bool>();


		} },
		{ "PointLight", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["PointLight"];
		PointLight& light = entity.AddComponent<PointLight>();

		light.color = comp["Color"].as<glm::vec3>();
		light.intensity = comp["Intensity"].as<float>();
		light.radius = comp["Radius"].as<float>();
		light.cast_shadows = comp["Cast Shadows"].as<bool>();


		} },
		{ "SpotLight", [](Entity entity, YAML::detail::iterator_value& value) {
		auto comp = value["SpotLight"];
		SpotLight& light = entity.AddComponent<SpotLight>();

		light.color = comp["Color"].as<glm::vec3>();
		light.intensity = comp["Intensity"].as<float>();
		light.innerCutOff = comp["innerCutOff"].as<float>();
		light.outerCutOff = comp["outerCutOff"].as<float>();
		if (comp["Cast Shadows"])
		{
			light.cast_shadows = comp["Cast Shadows"].as<bool>();
		}


		} },
		{ "AnimationGraphController", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["AnimationGraphController"];
			AnimationGraphController& controller = entity.AddComponent<AnimationGraphController>();

			if (comp["file id"])
			{
				UUID id = comp["file id"].as<UUID>();
				Ref<Animation::Graph> result = AnimationsSerializer::DeserializeAnimGraph(GetPathFromUUID(id).string());
				controller.graph.CreateInstance(result, entity.GetScene(), entity.GetID());
			}
			if (comp["Play On Start"])
			{
				controller.play_on_start = comp["Play On Start"].as<bool>();
			}
		} },
		{ "SequencePlayer", [](Entity entity, YAML::detail::iterator_value& value) {
			auto comp = value["SequencePlayer"];
			SequencePlayer& player = entity.AddComponent<SequencePlayer>();

			if (comp["file id"])
			{
				UUID id = comp["file id"].as<UUID>();
				Ref<Animation::Sequence> result = AnimationsSerializer::DeserializeSequence(GetPathFromUUID(id).string());
				player.sequence.CreateInstance(result, entity.GetScene());
			}
			if (comp["Play On Start"])
			{
				player.play_on_start = comp["Play On Start"].as<bool>();
			}
		} },
	};


	static bool SerializeEntites(Entity entity, YAML::Emitter& emit)
	{
		IDComponent& uuid = entity.GetComponent<IDComponent>();

		emit << YAML::Key << "UUID" << YAML::Value << uuid._id;

		for (uint32_t i = 0; i < ARRAYSIZE(_serialize_components); i++)
		{
			_serialize_components[i](entity, emit);
		}

		return true;
	}

	
	static void serialize_entity(Entity entity, YAML::Emitter& emit, Scene* scene)
	{
		emit << YAML::BeginMap;
		SerializeEntites(entity, emit);


		serialize_entity_scripts(entity, emit, scene);

		emit << YAML::EndMap;
	}
	static Entity deserialize_entity(Scene* scene, YAML::detail::iterator_value& entity)
	{
		UUID uuid = entity["UUID"].as<uint64_t>();
		UUID parent = 0;
		if (entity["Parent"])
		{
			parent = entity["Parent"].as<uint64_t>();
		}
		Entity obj = scene->CreateEntity_UUID(uuid, "", parent);
		Entity parent_ent = scene->GetEntity(parent);
		for (auto& i : _deserialize_components)
		{
			if (entity[i.first]) i.second(obj, entity);
		}

		deserialize_entity_scripts(obj, scene, entity);

		HierachyComponent& hi = obj.GetComponent<HierachyComponent>();
		if (entity["Is Enabled"])
		{
			hi.is_enabled = entity["Is Enabled"].as<bool>();
		}


		return obj;
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
		emit << YAML::Key << "Prefab Name" << YAML::Value << prefab->GetName();

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
		uint32_t prefab_str_count = static_cast<uint32_t>(prefab->GetName().size() + 1);
		stream->Write<uint32_t>(prefab_str_count);
		stream->Write((void*)prefab->GetName().data(), prefab_str_count);
		
		uint32_t pos_1 = stream->GetPosition();
		uint32_t entity_count = 0;
		stream->Write<uint32_t>(entity_count);

		Scene* scene = PrefabManager::get_instance()->GetScene();
		Entity handle = scene->GetEntity(prefab->GetHandle());
		serialize_entity_components_binary(handle, stream, handle.GetScene());


		for (auto& i : handle.GetComponent<HierachyComponent>().children)
		{
			Entity child = handle.GetScene()->GetEntity(i);
			SerializePrefabEntity_Binary(child, stream, entity_count);
			entity_count++;
		}

		uint32_t pos_2 = stream->GetPosition();
		stream->SetPosition(pos_1);
		stream->Write<uint32_t>(entity_count);
		stream->SetPosition(pos_2);

		return true;
	}

	Ref<Scene> SceneSerializer::Deserialize(const std::string& file_path)
	{
		YAML::Node data;
		YAML::load_yaml_data(file_path, data);
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
		if (!data["Trace Version"] || !data["Prefab Name"])
		{
			TRC_ERROR("These file is not a valid Prefab file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string prefab_version = data["Prefab Version"].as<std::string>(); // TODO: To be used later
		std::string prefab_name = data["Prefab Name"].as<std::string>();

		Ref<Prefab> prefab = PrefabManager::get_instance()->Get(prefab_name);
		if (prefab) return prefab;

		prefab = PrefabManager::get_instance()->Create(prefab_name);

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

		result = prefab;

		return result;
	}

	Ref<Prefab> SceneSerializer::DeserializePrefab(DataStream* stream)
	{
		Ref<Prefab> result;

		uint32_t prefab_str_count = 0;
		std::string prefab_name;
		stream->Read<uint32_t>(prefab_str_count);
		prefab_name.reserve(prefab_str_count);
		prefab_name.resize(prefab_str_count - 1);
		stream->Read((void*)prefab_name.data(), prefab_str_count);
		uint32_t entity_count = 0;
		stream->Read<uint32_t>(entity_count);
		Ref<Prefab> prefab = PrefabManager::get_instance()->Get(prefab_name);
		if (prefab)
		{
			return prefab;
		}

		prefab = PrefabManager::get_instance()->Create(prefab_name);


		Scene* scene = PrefabManager::get_instance()->GetScene();

		Entity obj;

		obj = deserialize_entity_components_binary(scene, stream);


		for (uint32_t i = 0; i < entity_count; i++)
		{
			deserialize_entity_components_binary(scene, stream);
		}

		prefab->SetHandle(obj.GetID());

		result = prefab;

		return result;
	}

	bool SceneSerializer::Deserialize(Ref<Scene> scene, FileStream& stream, AssetHeader& header)
	{
		if (!scene)
		{
			TRC_WARN("Invalid scene, function -> {}", __FUNCTION__);
			return false;
		}

		stream.SetPosition(header.offset);
		int file_lenght = header.data_size;
		char* _data = new char[file_lenght];//TODO: Use custom allocator
		stream.Read(_data, file_lenght);
		std::string file_data = _data;


		YAML::Node data = YAML::Load(file_data);
		if (!data["Trace Version"] || !data["Scene Name"])
		{
			return Ref<Scene>();
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string scene_version = data["Scene Version"].as<std::string>(); // TODO: To be used later
		std::string scene_name = data["Scene Name"].as<std::string>();

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
						auto f_it = ins.GetFields().find(field_name);
						if (f_it == ins.GetFields().end())
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

		delete[] _data;//TODO: Use custom allocator

		return true;
	}

	bool SceneSerializer::Serialize(Ref<Scene> scene, DataStream* stream)
	{
		uint32_t scene_str_count = static_cast<uint32_t>(scene->GetName().size() + 1);
		stream->Write<uint32_t>(scene_str_count);
		stream->Write((void*)scene->GetName().data(), scene_str_count);

		uint32_t pos_1 = stream->GetPosition();
		uint32_t entity_count = 0;
		stream->Write<uint32_t>(entity_count);
		auto process_hierachy = [&](Entity entity, UUID, Scene*)
		{
			Entity en(entity, scene.get());
			serialize_entity_components_binary(en, stream, scene.get());
			entity_count++;
		};

		scene->ProcessEntitiesByHierachy(process_hierachy, false);
		uint32_t pos_2 = stream->GetPosition();
		stream->SetPosition(pos_1);
		stream->Write<uint32_t>(entity_count);
		stream->SetPosition(pos_2);

		return true;
	}

	Ref<Scene> SceneSerializer::Deserialize(DataStream* stream)
	{
		uint32_t scene_str_count = 0;
		std::string scene_name;
		stream->Read<uint32_t>(scene_str_count);
		scene_name.reserve(scene_str_count);
		scene_name.resize(scene_str_count - 1);
		stream->Read((void*)scene_name.data(), scene_str_count);
		Ref<Scene> scene = SceneManager::get_instance()->GetScene(scene_name);
		if (scene)
		{
			TRC_WARN("{} has already been loaded", scene_name);
			return scene;
		}
		scene = SceneManager::get_instance()->CreateScene(scene_name);
		uint32_t entity_count = 0;
		stream->Read<uint32_t>(entity_count);
		for (uint32_t i = 0; i < entity_count; i++)
		{
			deserialize_entity_components_binary(scene.get(), stream);
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


		auto img_lambda = [&](Entity entity) {
			ImageComponent& img = entity.GetComponent<ImageComponent>();
			if (!img.image)
			{
				return;
			}
			UUID id = GetUUIDFromName(img.image->GetName());
			auto it = map.find(id);
			
			if (it == map.end())
			{
				Ref<GTexture> res = img.image;
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

		auto model_lambda = [&](Entity entity) {
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return;
			}
			Ref<MaterialInstance> res = renderer._material;
			for (auto& m_data : res->GetMaterialData())
			{
				trace::UniformMetaData& meta_data = res->GetRenderPipline()->GetSceneUniforms()[m_data.second.second];
				if (meta_data.data_type == ShaderData::CUSTOM_DATA_TEXTURE)
				{
					Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(m_data.second.first);
					UUID id = GetUUIDFromName(tex->GetName());

					auto it = map.find(id);

					if (it == map.end())
					{

						TextureDesc tex_desc = tex->GetTextureDescription();
						uint32_t tex_size = tex_desc.m_width * tex_desc.m_height * getFmtSize(tex_desc.m_format);
						TRC_INFO("Texture Name: {}, Texture Size: {}", res->GetName(), tex_size);
						total_tex_size += (float)tex_size;
						if (data_size < tex_size)
						{
							if (data) delete[] data;// TODO: Use custom allocator
							data = new char[tex_size];
							data_size = tex_size;
						}
						RenderFunc::GetTextureData(tex.get(), (void*&)data);
						AssetHeader ast_h;
						ast_h.offset = stream.GetPosition();
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

				}
			}
		};

		auto skinned_model_lambda = [&](Entity entity) {
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return;
			}
			Ref<MaterialInstance> res = renderer._material;
			for (auto& m_data : res->GetMaterialData())
			{
				trace::UniformMetaData& meta_data = res->GetRenderPipline()->GetSceneUniforms()[m_data.second.second];
				if (meta_data.data_type == ShaderData::CUSTOM_DATA_TEXTURE)
				{
					Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(m_data.second.first);
					UUID id = GetUUIDFromName(tex->GetName());

					auto it = map.find(id);

					if (it == map.end())
					{

						TextureDesc tex_desc = tex->GetTextureDescription();
						uint32_t tex_size = tex_desc.m_width * tex_desc.m_height * getFmtSize(tex_desc.m_format);
						TRC_INFO("Texture Name: {}, Texture Size: {}", res->GetName(), tex_size);
						total_tex_size += (float)tex_size;
						if (data_size < tex_size)
						{
							if (data) delete[] data;// TODO: Use custom allocator
							data = new char[tex_size];
							data_size = tex_size;
						}
						RenderFunc::GetTextureData(tex.get(), (void*&)data);
						AssetHeader ast_h;
						ast_h.offset = stream.GetPosition();
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

				}
			}
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

		auto anim_lambda = [&](Entity entity) 
		{
			AnimationComponent& anim = entity.GetComponent<AnimationComponent>();
			if (!anim.animation)
			{
				return;
			}
			UUID id = GetUUIDFromName(anim.animation->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			AnimationsSerializer::SerializeAnimationClip(anim.animation, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

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

		auto graph_lambda = [&](Entity entity) 
		{
			AnimationGraphController& anim = entity.GetComponent<AnimationGraphController>();
			Ref<Animation::Graph> graph = anim.graph.GetGraph();
			if (!graph)
			{
				return;
			}
			uint64_t anim_clip_type_id = Reflection::TypeID<Ref<AnimationClip>>();
			Reflection::CustomMemberCallback(*graph.get(), anim_clip_type_id, anim_clip_callback);

		};

		scene->IterateComponent<AnimationGraphController>(graph_lambda);

		auto sequence_lambda = [&](Entity entity)
		{
			SequencePlayer& player = entity.GetComponent<SequencePlayer>();
			Ref<Animation::Sequence> sequence = player.sequence.GetSequence();
			if (!sequence)
			{
				return;
			}
			uint64_t anim_clip_type_id = Reflection::TypeID<Ref<AnimationClip>>();
			Reflection::CustomMemberCallback(*sequence.get(), anim_clip_type_id, anim_clip_callback);

		};

		scene->IterateComponent<SequencePlayer>(sequence_lambda);

		return true;
	}

	bool SceneSerializer::SerializeAnimationGraphs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		auto graph_lambda = [&](Entity entity)
		{
			AnimationGraphController& anim = entity.GetComponent<AnimationGraphController>();
			Ref<Animation::Graph> graph = anim.graph.GetGraph();
			if (!graph)
			{
				return;
			}
			
			UUID id = GetUUIDFromName(graph->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();

			AnimationsSerializer::SerializeAnimGraph(graph, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

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
		
		auto fnt_lambda = [&](Entity entity)
		{
			TextComponent& txt = entity.GetComponent<TextComponent>();
			if (!txt.font)
			{
				return;
			}
			UUID id = GetUUIDFromName(txt.font->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}

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

				map.emplace(std::make_pair(id, ast_h));
			}
			else
			{
				TRC_ERROR("Failed to open font file, path -> {}", p.string());
			}
			
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
		
		auto mdl_lambda = [&](Entity entity)
		{
			ModelComponent& comp = entity.GetComponent<ModelComponent>();
			if (comp._model)
			{
				return;
			}
			UUID id = GetUUIDFromName(comp._model->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			Ref<Model> res = comp._model;
			AssetHeader ast_h;
			ast_h.offset = stream.GetPosition();
			std::string model_name = res->GetName();
			Reflection::Serialize(model_name, &stream, nullptr, Reflection::SerializationFormat::BINARY);
			Reflection::Serialize(*res.get(), &stream, nullptr, Reflection::SerializationFormat::BINARY);
			ast_h.data_size = stream.GetPosition() - ast_h.offset;

			map.emplace(std::make_pair(id, ast_h));
		};

		scene->IterateComponent<ModelComponent>(mdl_lambda);


		return true;
	}

	bool SceneSerializer::SerializeMaterials(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		auto mat_lambda = [&](Entity entity)
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return;
			}
			UUID id = GetUUIDFromName(renderer._material->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			MaterialSerializer::Serialize(renderer._material, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity)
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return;
			}
			UUID id = GetUUIDFromName(renderer._material->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();
			MaterialSerializer::Serialize(renderer._material, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));
		};

		scene->IterateComponent<SkinnedModelRenderer>(skin_mat_lambda);

		return true;
	}

	bool SceneSerializer::SerializePipelines(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		
		auto mat_lambda = [&](Entity entity)
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return;
			}
			UUID id = GetUUIDFromName(pipeline->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();

			PipelineSerializer::Serialize(pipeline, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity)
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return;
			}
			UUID id = GetUUIDFromName(pipeline->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}
			AssetHeader header = {};
			header.offset = stream.GetPosition();

			PipelineSerializer::Serialize(pipeline, &stream);
			header.data_size = stream.GetPosition() - header.offset;
			map.emplace(std::make_pair(id, header));
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

			Ref<GShader> shader_ref = ShaderManager::get_instance()->GetShader(shader->GetName());

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

		auto mat_lambda = [&](Entity entity)
		{
			ModelRendererComponent& renderer = entity.GetComponent<ModelRendererComponent>();
			if (!renderer._material)
			{
				return;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return;
			}
			PipelineStateDesc ds = pipeline->GetDesc();
			shad_lambda(ds.vertex_shader);
			shad_lambda(ds.pixel_shader);
		};

		scene->IterateComponent<ModelRendererComponent>(mat_lambda);

		auto skin_mat_lambda = [&](Entity entity)
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._material)
			{
				return;
			}
			Ref<GPipeline> pipeline = renderer._material->GetRenderPipline();
			if (!pipeline)
			{
				return;
			}
			PipelineStateDesc ds = pipeline->GetDesc();
			shad_lambda(ds.vertex_shader);
			shad_lambda(ds.pixel_shader);
		};

		scene->IterateComponent<SkinnedModelRenderer>(skin_mat_lambda);

		return true;
	}

	bool SceneSerializer::SerializePrefabs(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{

		auto prefab_lambda = [&](Entity entity)
		{
			PrefabComponent& comp = entity.GetComponent<PrefabComponent>();
			Ref<Prefab> prefab = comp.handle;
			if (!prefab)
			{
				return;
			}

			UUID id = GetUUIDFromName(prefab->GetName());
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

		scene->IterateComponent<PrefabComponent>(prefab_lambda);

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

		auto graph_lambda = [&](Entity entity)
		{
			AnimationGraphController& anim = entity.GetComponent<AnimationGraphController>();
			Ref<Animation::Graph> graph = anim.graph.GetGraph();
			if (!graph)
			{
				return;
			}
			uint64_t skeleton_type_id = Reflection::TypeID<Ref<Animation::Skeleton>>();
			Reflection::CustomMemberCallback(*graph.get(), skeleton_type_id, skeleton_callback);

		};

		scene->IterateComponent<AnimationGraphController>(graph_lambda);

		auto skinned_model_lambda = [&](Entity entity)
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			uint64_t skeleton_type_id = Reflection::TypeID<Ref<Animation::Skeleton>>();
			Reflection::CustomMemberCallback(renderer, skeleton_type_id, skeleton_callback);

		};

		scene->IterateComponent<SkinnedModelRenderer>(skinned_model_lambda);


		return true;
	}

	bool SceneSerializer::SerializeSequences(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		auto sequence_lambda = [&](Entity entity)
		{
			SequencePlayer& anim = entity.GetComponent<SequencePlayer>();
			Ref<Animation::Sequence> sequence = anim.sequence.GetSequence();
			if (!sequence)
			{
				return;
			}

			UUID id = GetUUIDFromName(sequence->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();

			AnimationsSerializer::SerializeSequence(sequence, &stream);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

		};

		scene->IterateComponent<SequencePlayer>(sequence_lambda);

		return true;
	}

	bool SceneSerializer::SerializeSkinnedModels(FileStream& stream, std::unordered_map<UUID, AssetHeader>& map, Ref<Scene> scene)
	{
		auto skinned_model_lambda = [&](Entity entity)
		{
			SkinnedModelRenderer& renderer = entity.GetComponent<SkinnedModelRenderer>();
			if (!renderer._model)
			{
				return;
			}
			UUID id = GetUUIDFromName(renderer._model->GetName());
			auto it = map.find(id);
			if (it != map.end())
			{
				return;
			}

			AssetHeader header = {};
			header.offset = stream.GetPosition();
			std::string model_name = renderer._model->GetName();
			Reflection::Serialize(model_name , &stream, nullptr, Reflection::SerializationFormat::BINARY);
			Reflection::Serialize(*renderer._model.get(), &stream, nullptr, Reflection::SerializationFormat::BINARY);
			header.data_size = stream.GetPosition() - header.offset;

			map.emplace(std::make_pair(id, header));

		};

		scene->IterateComponent<SkinnedModelRenderer>(skinned_model_lambda);

		return true;
	}

}