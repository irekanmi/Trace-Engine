#include "pch.h"

#include "ShaderManager.h"
#include "render/Renderutils.h"
#include "render/ShaderParser.h"

namespace trace {

	ShaderManager* ShaderManager::s_instance = nullptr;

	bool ShaderManager::Init(uint32_t max_shader_units)
	{
		m_numShaderUnits = max_shader_units;

		m_hashTable.Init(m_numShaderUnits);
		m_hashTable.Fill(INVALID_ID);

		m_shaders.resize(m_numShaderUnits);


		for (uint32_t i = 0; i < m_numShaderUnits; i++)
		{
			m_shaders[i].m_id = INVALID_ID;
		}

		// Trying to determine directory where shaders are located..............
		std::filesystem::path current_search_path("./assets");

		if (std::filesystem::exists(current_search_path))
		{
			current_search_path /= "shaders";
			if (std::filesystem::exists(current_search_path))
			{
				shader_resource_path = current_search_path;
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "shaders";
				if (std::filesystem::exists(current_search_path))
				{
					shader_resource_path = current_search_path;
				}
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "shaders";
				if (std::filesystem::exists(current_search_path))
				{
					shader_resource_path = current_search_path;
				}
			}
		}
		else
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "shaders";
				if (std::filesystem::exists(current_search_path))
				{
					shader_resource_path = current_search_path;
				}
			}
		}
		// .............................

		return true;
	}

	void ShaderManager::ShutDown()
	{

		for (GShader& i : m_shaders)
		{
			if(i.m_id != INVALID_ID)
				RenderFunc::DestroyShader(&i);
		}

		m_shaders.clear();

	}

	Ref<GShader> ShaderManager::GetShader(const std::string& name)
	{
		Ref<GShader> result;
		GShader* _shad = nullptr;
		uint32_t hash = m_hashTable.Get(name);
		if (hash == INVALID_ID)
		{
			TRC_WARN("{} Shader has not been created yet", name);
			return result;
		}
		_shad = &m_shaders[hash];
		result = { _shad, BIND_RENDER_COMMAND_FN(ShaderManager::UnloadShader) };
		return result;
	}

	Ref<GShader> ShaderManager::CreateShader(const std::string& name, ShaderStage shader_stage)
	{
		return CreateShader_((shader_resource_path/name).string(), shader_stage);
	}

	Ref<GShader> ShaderManager::CreateShader_(const std::string& path, ShaderStage shader_stage)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		Ref<GShader> result;
		GShader* _shad = nullptr;
		if (m_hashTable.Get(name) != INVALID_ID)
		{
			TRC_WARN("Shader {} already exists", name);
			return GetShader(name);
		}

		uint32_t i = 0;
		for (GShader& shader : m_shaders)
		{
			if (shader.m_id == INVALID_ID)
			{
				std::string shader_data = ShaderParser::load_shader_file(path);
				if (RenderFunc::CreateShader(&shader, shader_data, shader_stage))
				{
					shader.m_id = i;
					m_hashTable.Set(name, i);
					shader.m_path = p;
					_shad = &shader;
					break;
				}
				TRC_ERROR("Failed to create shader => {}", name);
				break;
			}
			i++;
		}

		result = { _shad, BIND_RENDER_COMMAND_FN(ShaderManager::UnloadShader) };
		return result;
	}

	void ShaderManager::UnloadShader(GShader* shader)
	{
	}

	ShaderManager* ShaderManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance =  new ShaderManager();
		}
		return s_instance;
	}

}