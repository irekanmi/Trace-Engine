#include "pch.h"

#include "SceneSerializer.h"
#include "Entity.h"
#include "Componets.h"
#include "core/FileSystem.h"

#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"

namespace trace {


	static bool SerializeEntites(Ref<Scene> scene, Entity entity, YAML::Emitter& emit)
	{
		emit << YAML::BeginMap;
		emit << YAML::Key << "UUID" << YAML::Value << INVALID_ID;

		if (entity.HasComponent<TagComponent>())
		{
			TagComponent& tag = entity.GetComponent<TagComponent>();
			emit << YAML::Key << "TagComponent" << YAML::Value;
			emit << YAML::BeginMap;
			emit << YAML::Key << "Tag" << YAML::Value << tag._tag;

			emit << YAML::EndMap;
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
		emit << YAML::Key << "Entites" << YAML::Value << YAML::BeginSeq;

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
		return Ref<Scene>();
	}

}