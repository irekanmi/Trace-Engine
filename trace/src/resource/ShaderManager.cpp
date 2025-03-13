#include "pch.h"

#include "ShaderManager.h"
#include "backends/Renderutils.h"
#include "render/ShaderParser.h"
#include "core/FileSystem.h"
#include "core/Utils.h"
#include "core/Coretypes.h"
#include "serialize/FileStream.h"
#include "serialize/PipelineSerializer.h"

namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);

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

		if (AppSettings::is_editor)
		{

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
		}

		return true;
	}

	void ShaderManager::ShutDown()
	{

		for (GShader& i : m_shaders)
		{
			if (i.m_id != INVALID_ID)
			{
				RenderFunc::DestroyShader(&i);
				TRC_TRACE("Shader was still in use, name : {}, RefCount : {}", i.GetName(), i.m_refCount);
			}
		}

		m_shaders.clear();

	}

	Ref<GShader> ShaderManager::GetShader(const std::string& name)
	{
		Ref<GShader> result;
		GShader* _shad = nullptr;
		uint32_t& hash = m_hashTable.Get_Ref(name);
		if (hash == INVALID_ID)
		{
			TRC_WARN("{} Shader has not been created yet", name);
			return result;
		}
		_shad = &m_shaders[hash];
		if (_shad->m_id == INVALID_ID)
		{
			TRC_WARN("{} shader has already been destroyed", name);
			hash = INVALID_ID;
			return result;
		}
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
		std::filesystem::path code_path = p;
		code_path += ".shcode";
		std::string name = p.filename().string();
		Ref<GShader> result;
		GShader* _shad = nullptr;
		result = GetShader(name);
		if (result)
		{
			TRC_WARN("Shader {} already exists", name);
			return result;
		}

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

		uint32_t i = 0;
		for (GShader& shader : m_shaders)
		{
			if (shader.m_id == INVALID_ID)
			{				
				if (RenderFunc::CreateShader(&shader, code, shader_stage))
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

		std::vector<std::pair<std::string, int>>& shader_data_index = _shad->GetDataIndex();
		shader_data_index = data_index;
		
		if (serialize)
		{
			PipelineSerializer::SerializeShader(_shad, code_path.string());
		}


		result = { _shad, BIND_RENDER_COMMAND_FN(ShaderManager::UnloadShader) };
		return result;
	}

	Ref<GShader> ShaderManager::CreateShader(std::vector<uint32_t>& code, std::vector<std::pair<std::string, int>>& data_index, const std::string& shader_name, ShaderStage shader_stage)
	{
		Ref<GShader> result;
		GShader* _shad = nullptr;
		result = GetShader(shader_name);
		if (result)
		{
			TRC_WARN("Shader {} already exists", shader_name);
			return result;
		}


		uint32_t i = 0;
		for (GShader& shader : m_shaders)
		{
			if (shader.m_id == INVALID_ID)
			{
				if (RenderFunc::CreateShader(&shader, code, shader_stage))
				{
					shader.m_id = i;
					m_hashTable.Set(shader_name, i);
					shader.m_path = shader_name;
					_shad = &shader;
					break;
				}
				TRC_ERROR("Failed to create shader => {}", shader_name);
				break;
			}
			i++;
		}

		std::vector<std::pair<std::string, int>>& shader_data_index = _shad->GetDataIndex();
		shader_data_index = std::move(data_index);

		result = { _shad, BIND_RENDER_COMMAND_FN(ShaderManager::UnloadShader) };
		return result;
	}

	void ShaderManager::UnloadShader(Resource* res)
	{
		GShader* shader = (GShader*)res;
		// TODO: Implement Shader Unloading
	}

	Ref<GShader> ShaderManager::LoadShader_Runtime(UUID id)
	{


		Ref<GShader> result;
		GShader* _shad = nullptr;

		auto it = m_assetMap.find(id);
		if (it == m_assetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}


		std::string bin_dir;
		FindDirectory(AppSettings::exe_path, "Data/trshd.trbin", bin_dir);
		FileStream stream(bin_dir, FileMode::READ);
		stream.SetPosition(it->second.offset);

		result = PipelineSerializer::DeserializeShader(&stream);
		return result;
	}

	void ShaderManager::RenameAsset(Ref<GShader> asset, const std::string& new_name)
	{
		uint32_t index = m_hashTable.Get(asset->GetName());
		m_hashTable.Set(asset->GetName(), INVALID_ID);
		m_hashTable.Set(new_name, index);
	}

	ShaderManager* ShaderManager::get_instance()
	{
		static ShaderManager* s_instance = new ShaderManager;
		return s_instance;
	}

	void ShaderManager::SaveShaderCode(std::filesystem::path& code_path, std::vector<uint32_t>& code)
	{
		FileHandle handle;
		std::string path = code_path.string();
		if (FileSystem::open_file(path, (FileMode)(FileMode::BINARY | FileMode::WRITE), handle))
		{
			uint32_t code_size = static_cast<uint32_t>(code.size() * 4);
			FileSystem::write(code.data(), code.size() * 4, handle);
		}
		FileSystem::close_file(handle);


	}

	std::vector<uint32_t> ShaderManager::LoadShaderCode(std::filesystem::path& code_path)
	{
		FileHandle handle;
		std::string path = code_path.string();
		std::vector<uint32_t> result;
		if (FileSystem::open_file(path, (FileMode)(FileMode::BINARY | FileMode::READ), handle))
		{
			uint32_t code_size = 0;
			FileSystem::read_all_bytes(handle, nullptr, code_size);
			result.resize(code_size / 4);
			FileSystem::read_all_bytes(handle, result.data(), code_size);
		}
		FileSystem::close_file(handle);

		return result;
	}

}