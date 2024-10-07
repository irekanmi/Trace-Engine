#include "pch.h"

#include "core/FileSystem.h"
#include "ProjectSerializer.h"
#include "serialize/FileStream.h"

#include "yaml_util.h"

namespace trace {
	bool ProjectSerializer::Serialize(Ref<Project> project, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Project Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Project Name" << YAML::Value << project->GetProjectName();

		if (project->GetStartScene() != 0)
		{
			emit << YAML::Key << "Start Scene" << YAML::Value << project->GetStartScene();
		}

		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, file_path);

		return true;
	}
	Ref<Project> ProjectSerializer::Deserialize(const std::string& file_path)
	{
		Ref<Project> result;


		YAML::Node data;
		YAML::load_yaml_data(file_path, data);

		if (!data["Trace Version"] || !data["Project Version"] || !data["Project Name"])
		{
			TRC_ERROR("These file is not a valid material file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string project_version = data["Project Version"].as<std::string>(); // TODO: To be used later
		std::string project_name = data["Project Name"].as<std::string>();
		std::filesystem::path p = std::filesystem::path(file_path).parent_path();

		Project* res = new Project(); // TODO: Use custom allocator
		res->SetName(project_name);
		if (data["Start Scene"])
		{
			UUID id = data["Start Scene"].as<uint64_t>();
			res->SetStartScene(id);
		}
		res->SetProjectCurrentDirectory(p.string());
		res->SetAssetsDirectory((p / "Assets").string());
		res->SetAssemblyPath((p / ("Data/Assembly/" + project_name + ".dll")).string());
		res->m_path = file_path;

		result = { res, [](Resource* proj) { delete proj;/*TODO: Use custom allocator*/ }};

		return result;
	}



	
}