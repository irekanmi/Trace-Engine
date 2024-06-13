#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include <vector>
#include <string>
#include "Graphics.h"

namespace trace {

	class GShader;

	enum ShaderLang
	{
		GLSL,
		HLSL,
		SPIRV
	};

	class TRACE_API ShaderParser
	{

	public:
		ShaderParser();
		~ShaderParser();

		static std::string spirv_to_glsl(std::vector<uint32_t> spir_v, ShaderStage shader_stage);
		static std::vector<uint32_t> glsl_to_spirv(const std::string& glsl, ShaderStage shader_stage, std::vector<std::pair<std::string, int>>& out_data_index);
		static std::string load_shader_file(const std::string& filename);
		static void generate_shader_resources(const std::string& shader_src, ShaderResources& out_res, ShaderStage shader_stage);
		static void generate_shader_resources(GShader* shader, ShaderResources& out_res);
		static void generate_shader_data_index(GShader* shader, std::vector<std::pair<std::string, int>>& out_index);

	private:
	protected:

	};

}
