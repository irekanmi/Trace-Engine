#include "pch.h"

#include "MaterialSerializer.h"
#include "resource/MaterialManager.h"
#include "core/FileSystem.h"


#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "yaml_util.h"

#include <string>



namespace trace {

	bool MaterialSerializer::Serialize(Ref<MaterialInstance> mat, const std::string& file_path)
	{
		YAML::Emitter emit;

		auto lambda = [&](trace::ShaderData type, std::any& dst, const std::string& name)
		{
			switch (type)
			{
			case trace::ShaderData::CUSTOM_DATA_BOOL:
			{
				bool* data = &std::any_cast<bool&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_FLOAT:
			{
				float* data = &std::any_cast<float&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_INT:
			{
				int* data = &std::any_cast<int&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC2:
			{
				glm::ivec2& data = std::any_cast<glm::ivec2&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC3:
			{
				glm::ivec3& data = std::any_cast<glm::ivec3&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC4:
			{
				glm::ivec4* data = &std::any_cast<glm::ivec4&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT2:
			{
				glm::mat2& data = std::any_cast<glm::mat2&>(dst);

				break;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT3:
			{
				glm::mat3& data = std::any_cast<glm::mat3&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT4:
			{
				glm::mat4& data = std::any_cast<glm::mat4&>(dst);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_TEXTURE:
			{
				Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2& data = std::any_cast<glm::vec2&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);

				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);

				break;
			}
			}
		};

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Material Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "Material Name" << YAML::Value << mat->GetName();
		emit << YAML::Key << "Data" << YAML::Value << YAML::BeginSeq;

		for (auto& data : mat->m_data)
		{
			emit << YAML::BeginMap;
			emit << YAML::Key << "Name" << YAML::Value << data.first;


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

		return result;
	}
}