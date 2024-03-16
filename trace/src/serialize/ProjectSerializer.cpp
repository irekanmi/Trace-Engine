#include "pch.h"

#include "core/FileSystem.h"
#include "ProjectSerializer.h"
#include "yaml_util.h"

namespace trace {
	bool ProjectSerializer::Serialize(Ref<Project> project, const std::string& file_path)
	{
		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Project Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Project Name" << YAML::Value << project->GetName();

		if (!project->GetStartScene().empty())
		{
			emit << YAML::Key << "Start Scene" << YAML::Value << project->GetStartScene();
		}

		emit << YAML::EndMap;

		FileHandle out_handle;
		if (FileSystem::open_file(file_path, FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, emit.c_str());
			FileSystem::close_file(out_handle);
		}

		return true;
	}
	Ref<Project> ProjectSerializer::Deserialize(const std::string& file_path)
	{
		Ref<Project> result;

		FileHandle in_handle;
		if (!FileSystem::open_file(file_path, FileMode::READ, in_handle))
		{
			TRC_ERROR("Unable to open file {}", file_path);
			return result;
		}
		std::string file_data;
		FileSystem::read_all_lines(in_handle, file_data);
		FileSystem::close_file(in_handle);

		YAML::Node data = YAML::Load(file_data);
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
		if (data["Start Scene"]) res->SetStartScene(data["Start Scene"].as<std::string>());
		res->current_directory = p.string();
		res->assets_directory = (p / "Assets").string();
		res->assembly_path = (p / ("Data/Assembly/" + project_name + ".dll")).string();

		result = { res, [](Project* proj) { delete proj;/*TODO: Use custom allocator*/ }};

		return result;
	}
}