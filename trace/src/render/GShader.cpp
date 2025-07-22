#include "pch.h"

#include "GShader.h"
#include "core/io/Logging.h"
#include "render/ShaderParser.h"
#include "serialize/PipelineSerializer.h"
#include "backends/Renderutils.h"
#include "resource/GenericAssetManager.h"
#include "resource/DefaultAssetsManager.h"
#include "external_utils.h"

#include <filesystem>


namespace trace {
	GShader::GShader()
	{
	}
	GShader::~GShader()
	{
	}

	bool GShader::Create(const std::string& path, ShaderStage shader_stage)
	{
		std::filesystem::path p(DefaultAssetsManager::assets_path + "/shaders/" + path);
		std::filesystem::path code_path = p;
		code_path += ".shcode";
		std::string name = p.filename().string();

		bool serialize = false;
		std::vector<uint32_t> code;
		std::vector<std::pair<std::string, int>> data_index;
		if (!std::filesystem::exists(code_path))
		{
			std::string shader_data = ShaderParser::load_shader_file(path);
			if (p.extension() == ".glsl")
			{
				code = ShaderParser::glsl_to_spirv(shader_data, shader_stage, data_index);
			}
			if (!code.empty())
			{
				//SaveShaderCode(code_path, code);
				serialize = true;
			}
		}
		else
		{
			//code = LoadShaderCode(code_path);
			PipelineSerializer::DeserializeShader(code_path.string(), code, data_index);
		}

		if (!RenderFunc::CreateShader(this, code, shader_stage))
		{
			TRC_ERROR("Failed to create shader => {}", GetName());
			return false;
		}

		std::vector<std::pair<std::string, int>>& shader_data_index = this->GetDataIndex();
		shader_data_index = data_index;

		if (serialize)
		{
			PipelineSerializer::SerializeShader(this, code_path.string());
		}

		return true;
	}

	bool GShader::Create(std::vector<uint32_t>& code, std::vector<std::pair<std::string, int>>& data_index, ShaderStage shader_stage)
	{

		if (!RenderFunc::CreateShader(this, code, shader_stage))
		{
			TRC_ERROR("Failed to create shader => {}", GetName());
			return false;
		}

		std::vector<std::pair<std::string, int>>& shader_data_index = this->GetDataIndex();
		shader_data_index = std::move(data_index);

		return true;
	}

	void GShader::Destroy()
	{
		RenderFunc::DestroyShader(this);
	}
	Ref<GShader> GShader::Deserialize(UUID id)
	{
		Ref<GShader> result;
		if (AppSettings::is_editor)
		{
			std::filesystem::path p = GetPathFromUUID(id).string();
			std::string name = p.filename().string();
			Ref<GShader> shader = GenericAssetManager::get_instance()->Get<GShader>(name);
			if (shader)
			{
				result = shader;
			}
			else
			{
				TRC_ASSERT(false, "These function has not been implemented");
			}
		}
		else
		{
			result = GenericAssetManager::get_instance()->Load_Runtime<GShader>(id);
		}
		return result;
	}

	Ref<GShader> GShader::Deserialize(DataStream* stream)
	{
		return PipelineSerializer::DeserializeShader(stream);
	}
}