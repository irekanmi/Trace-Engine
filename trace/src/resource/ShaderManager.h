#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/GShader.h"
#include "HashTable.h"
#include <string>
#include <filesystem>

namespace trace {

	class TRACE_API ShaderManager
	{

	public:
		ShaderManager() {}
		~ShaderManager() {}

		bool Init(uint32_t max_shader_units);
		void ShutDown();

		GShader* GetShader(const std::string& name);
		bool CreateShader(const std::string& name, ShaderStage shader_stage);
		void UnloadShader(GShader* shader);
		std::string GetShaderResourcePath() { return shader_resource_path.string(); }

		static ShaderManager* get_instance();

	private:
		uint32_t m_numShaderUnits;
		std::vector<GShader> m_shaders;
		HashTable<uint32_t> m_hashTable;
		std::filesystem::path shader_resource_path;

		static ShaderManager* s_instance;
		

	protected:

	};

}