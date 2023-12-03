#include "pch.h"

#include "MaterialSerializer.h"
#include "resource/MaterialManager.h"
#include "resource/PipelineManager.h"
#include "resource/TextureManager.h"
#include "core/FileSystem.h"
#include "backends/Renderutils.h"
#include "scene/UUID.h"


#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "yaml_util.h"
#include "PipelineSerializer.h"

#include <string>



namespace trace {

	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	bool MaterialSerializer::Serialize(Ref<MaterialInstance> mat, const std::string& file_path)
	{
		YAML::Emitter emit;

		auto lambda = [](YAML::Emitter& emit,trace::ShaderData type, std::any& dst)
		{
			switch (type)
			{
			case trace::ShaderData::CUSTOM_DATA_BOOL:
			{
				bool* data = &std::any_cast<bool&>(dst);
				emit << *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_FLOAT:
			{
				float* data = &std::any_cast<float&>(dst);
				emit << *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_INT:
			{
				int* data = &std::any_cast<int&>(dst);
				emit << *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC2:
			{
				glm::ivec2& data = std::any_cast<glm::ivec2&>(dst);
				emit << data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC3:
			{
				glm::ivec3& data = std::any_cast<glm::ivec3&>(dst);
				emit << data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC4:
			{
				glm::ivec4* data = &std::any_cast<glm::ivec4&>(dst);
				emit << data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT2:
			{
				glm::mat2& data = std::any_cast<glm::mat2&>(dst);
				TRC_ASSERT(false, "Function has not been implemented, {}, line -> {}", __FUNCTION__, __LINE__);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT3:
			{
				glm::mat3& data = std::any_cast<glm::mat3&>(dst);
				TRC_ASSERT(false, "Function has not been implemented, {}, line -> {}", __FUNCTION__, __LINE__);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT4:
			{
				glm::mat4& data = std::any_cast<glm::mat4&>(dst);
				TRC_ASSERT(false, "Function has not been implemented, {}, line -> {}", __FUNCTION__, __LINE__);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_TEXTURE:
			{
				Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(dst);
				emit << YAML::BeginMap;
				std::string filename = tex->m_path.filename().string();
				emit << YAML::Key << "file_id" << YAML::Value << GetUUIDFromName(filename);
				emit << YAML::EndMap;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2& data = std::any_cast<glm::vec2&>(dst);
				emit << data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);
				emit << data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);
				emit << data;
				break;
			}
			}
		};

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Material Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Material Name" << YAML::Value << mat->GetName();
		emit << YAML::Key << "Pipeline ID" << YAML::Value << GetUUIDFromName(mat->GetRenderPipline()->GetName());
		emit << YAML::Key << "Data" << YAML::Value << YAML::BeginSeq;

		for (auto& data : mat->m_data)
		{
			trace::UniformMetaData& meta_data = mat->m_renderPipeline->Scene_uniforms[data.second.second];
			emit << YAML::BeginMap;
			emit << YAML::Key << "Name" << YAML::Value << data.first;
			emit << YAML::Key << "Type" << YAML::Value << (int)meta_data.data_type;
			emit << YAML::Key << "Value" << YAML::Value;
			lambda(emit, meta_data.data_type, data.second.first);

			emit << YAML::EndMap;
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

	Ref<MaterialInstance> MaterialSerializer::Deserialize(const std::string& file_path)
	{
		Ref<MaterialInstance> result;

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
		if (!data["Trace Version"] || !data["Material Version"] || !data["Material Name"])
		{
			TRC_ERROR("These file is not a valid material file {}", file_path);
			return result;
		}

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
		std::string material_version = data["Material Version"].as<std::string>(); // TODO: To be used later
		std::string material_name = data["Material Name"].as<std::string>();

		result = MaterialManager::get_instance()->GetMaterial(material_name);
		if (result)
		{
			TRC_WARN("{} has already been loaded", material_name);
			return result;
		}

		auto lambda = [](YAML::detail::iterator_value& value, trace::ShaderData type, std::any& dst)
		{
			switch (type)
			{
			case trace::ShaderData::CUSTOM_DATA_BOOL:
			{
				bool data = value["Value"].as<bool>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_FLOAT:
			{
				float data = value["Value"].as<float>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_INT:
			{
				int data = value["Value"].as<int>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC2:
			{
				glm::ivec2 data = value["Value"].as<glm::ivec2>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC3:
			{
				glm::ivec3 data = value["Value"].as<glm::ivec3>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC4:
			{
				glm::ivec4 data = value["Value"].as<glm::ivec4>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT2:
			{
				TRC_ASSERT(false, "Function has not been implemented, {}, line -> {}", __FUNCTION__, __LINE__);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT3:
			{
				TRC_ASSERT(false, "Function has not been implemented, {}, line -> {}", __FUNCTION__, __LINE__);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT4:
			{
				TRC_ASSERT(false, "Function has not been implemented, {}, line -> {}", __FUNCTION__, __LINE__);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_TEXTURE:
			{
				auto info = value["Value"];
				UUID id = info["file_id"].as<uint64_t>();
				Ref<GTexture> tex;
				std::filesystem::path p = GetPathFromUUID(id);
				tex = TextureManager::get_instance()->GetTexture(p.filename().string());
				if(!tex) tex = TextureManager::get_instance()->LoadTexture_(p.string());
				if (tex)
					dst = tex;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2 data = value["Value"].as<glm::vec2>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3 data = value["Value"].as<glm::vec3>();
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4 data = value["Value"].as<glm::vec4>();
				dst = data;
				break;
			}
			}
		};

		std::filesystem::path pipe_path = GetPathFromUUID(data["Pipeline ID"].as<uint64_t>());
		std::string pipe_name = pipe_path.filename().string();
		Ref<GPipeline> pipeline = PipelineManager::get_instance()->GetPipeline(pipe_name);

		if (!pipeline)
		{
			pipeline = PipelineSerializer::Deserialize(pipe_path.string());
		}

		if(!pipeline)
		{
			TRC_WARN("{} pipeline is not valid, please ensure to pass a valid pipeline", pipe_name);
			return result;
		}

		result = MaterialManager::get_instance()->CreateMaterial(material_name, pipeline);
		YAML::Node res = data["Data"];
		if (res)
		{
			for (auto val : res)
			{
				std::string name = val["Name"].as<std::string>();
				auto it = result->m_data.find(name);
				ShaderData type = (ShaderData)val["Type"].as<int>();
				if (it != result->m_data.end())
				{
					lambda(val, type, it->second.first);
				}
			}
		}
		RenderFunc::PostInitializeMaterial(result.get(), pipeline);

		return result;
	}
}