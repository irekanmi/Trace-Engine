#include "pch.h"

#include "MaterialSerializer.h"
#include "resource/MaterialManager.h"
#include "resource/PipelineManager.h"
#include "resource/TextureManager.h"
#include "core/FileSystem.h"
#include "backends/Renderutils.h"
#include "scene/UUID.h"
#include "external_utils.h"


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

		for (auto& data : mat->GetMaterialData())
		{
			trace::UniformMetaData& meta_data = mat->GetRenderPipline()->GetSceneUniforms()[data.second.second];
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

	/*
	* Material
	*  '-> pipeline_id
	*  '-> data_count
	*   for each data
	*    '-> name_length
	*    '-> name_data
	*    '-> value_size
	*    '-> value_data
	*/
	bool MaterialSerializer::Serialize(Ref<MaterialInstance> material, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{
		if (!material)
		{
			TRC_WARN("Pass in a valid material, Function -> {}", __FUNCTION__);
			return false;
		}


		UUID id = GetUUIDFromName(material->GetName());
		auto it = std::find_if(map.begin(), map.end(), [&id](std::pair<UUID, AssetHeader>& i)
			{
				return i.first == id;
			});

		if (it == map.end())
		{
			AssetHeader ast_h;
			ast_h.offset = stream.GetPosition();
			uint64_t pipeline_id = GetUUIDFromName(material->GetRenderPipline()->GetName());
			stream.Write<uint64_t>(pipeline_id);
			uint32_t data_count = material->GetMaterialData().size();
			stream.Write<uint32_t>(data_count);
			auto lambda = [](FileStream& stream, trace::ShaderData type, std::any& dst)
			{
				
				switch (type)
				{
				case trace::ShaderData::CUSTOM_DATA_BOOL:
				{
					bool* data = &std::any_cast<bool&>(dst);
					uint16_t data_size = sizeof(bool);
					stream.Write(data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_FLOAT:
				{
					float* data = &std::any_cast<float&>(dst);
					uint16_t data_size = sizeof(float);
					stream.Write(data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_INT:
				{
					int* data = &std::any_cast<int&>(dst);
					uint16_t data_size = sizeof(int);
					stream.Write(data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_IVEC2:
				{
					glm::ivec2& data = std::any_cast<glm::ivec2&>(dst);
					uint16_t data_size = sizeof(glm::ivec2);
					stream.Write(&data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_IVEC3:
				{
					glm::ivec3& data = std::any_cast<glm::ivec3&>(dst);
					uint16_t data_size = sizeof(glm::ivec3);
					stream.Write(&data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_IVEC4:
				{
					glm::ivec4* data = &std::any_cast<glm::ivec4&>(dst);
					uint16_t data_size = sizeof(glm::ivec4);
					stream.Write(&data, data_size);
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
					uint16_t data_size = sizeof(uint64_t);
					uint64_t data = GetUUIDFromName(tex->GetName());
					stream.Write(&data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_VEC2:
				{
					glm::vec2& data = std::any_cast<glm::vec2&>(dst);
					uint16_t data_size = sizeof(glm::vec2);
					stream.Write(&data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_VEC3:
				{
					glm::vec3& data = std::any_cast<glm::vec3&>(dst);
					uint16_t data_size = sizeof(glm::vec3);
					stream.Write(&data, data_size);
					break;
				}
				case trace::ShaderData::CUSTOM_DATA_VEC4:
				{
					glm::vec4& data = std::any_cast<glm::vec4&>(dst);
					uint16_t data_size = sizeof(glm::vec4);
					stream.Write(&data, data_size);
					break;
				}
				}
			};
			for (auto& i : material->GetMaterialData())
			{
				uint32_t name_length = i.first.length() + 1;
				stream.Write<uint32_t>(name_length);
				stream.Write((void*)i.first.data(), name_length);
				trace::UniformMetaData& meta_data = material->GetRenderPipline()->GetSceneUniforms()[i.second.second];
				int type = (int)meta_data.data_type;
				stream.Write<int>(type);
				lambda(stream, meta_data.data_type, i.second.first);
			}
			ast_h.data_size = stream.GetPosition() - ast_h.offset;

			map.push_back(std::make_pair(id, ast_h));
		}


		return true;
	}

	Ref<MaterialInstance> MaterialSerializer::Deserialize(const std::string& file_path)
	{
		Ref<MaterialInstance> result;

		std::filesystem::path p = file_path;
		result = MaterialManager::get_instance()->GetMaterial(p.filename().string());
		if (result)
		{
			TRC_WARN("{} has already been loaded", p.filename().string());
			return result;
		}

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
				Ref<GTexture> tex = LoadTexture(id);

				if (tex)
				{
					dst = tex;
				}
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
				auto it = result->GetMaterialData().find(name);
				ShaderData type = (ShaderData)val["Type"].as<int>();
				if (it != result->GetMaterialData().end())
				{
					lambda(val, type, it->second.first);
				}
			}
		}
		RenderFunc::PostInitializeMaterial(result.get(), pipeline);

		return result;
	}
	bool MaterialSerializer::Deserialize(Ref<GPipeline> pipeline, MaterialInstance* material, MemoryStream& stream)
	{
		int data_count = 0;
		stream.Read<int>(data_count);


		auto lambda = [](MemoryStream& stream, trace::ShaderData type, std::any& dst)
		{

			switch (type)
			{
			case trace::ShaderData::CUSTOM_DATA_BOOL:
			{
				bool* data = &std::any_cast<bool&>(dst);
				stream.Read(data);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_FLOAT:
			{
				float* data = &std::any_cast<float&>(dst);
				uint16_t data_size = sizeof(float);
				stream.Read(data, data_size);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_INT:
			{
				int* data = &std::any_cast<int&>(dst);
				uint16_t data_size = sizeof(int);
				stream.Read(data, data_size);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC2:
			{
				glm::ivec2& data = std::any_cast<glm::ivec2&>(dst);
				uint16_t data_size = sizeof(glm::ivec2);
				stream.Read(&data, data_size);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC3:
			{
				glm::ivec3& data = std::any_cast<glm::ivec3&>(dst);
				uint16_t data_size = sizeof(glm::ivec3);
				stream.Read(&data, data_size);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC4:
			{
				glm::ivec4* data = &std::any_cast<glm::ivec4&>(dst);
				uint16_t data_size = sizeof(glm::ivec4);
				stream.Read(&data, data_size);
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
				UUID tex_id = 0;
				stream.Read<UUID>(tex_id);
				Ref<GTexture> tex = TextureManager::get_instance()->LoadTexture_Runtime(tex_id);
				dst = tex;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2& data = std::any_cast<glm::vec2&>(dst);
				uint16_t data_size = sizeof(glm::vec2);
				stream.Read(&data, data_size);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);
				uint16_t data_size = sizeof(glm::vec3);
				stream.Read(&data, data_size);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);
				uint16_t data_size = sizeof(glm::vec4);
				stream.Read(&data, data_size);
				break;
			}
			}
		};

		char buf[512] = { 0 };
		for (int i = 0; i < data_count; i++)
		{
			int name_length = 0;
			stream.Read<int>(name_length);
			stream.Read(buf, name_length);
			std::string name = buf;
			int type = -1;
			stream.Read<int>(type);

			auto& data = material->GetMaterialData();
			auto it = data.find(name);
			if (it != data.end())
			{
				lambda(stream, (ShaderData)type, it->second.first);
			}

		}

		return true;
	}
}