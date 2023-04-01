#include "pch.h"

#include "ShaderParser.h"
#include "shaderc/shaderc.hpp"
#include "core/io/Logging.h"
#include "core/FileSystem.h"

shaderc_shader_kind convertToShadercFmt(trace::ShaderStage stage, trace::ShaderLang lang);

namespace trace {
	ShaderParser::ShaderParser()
	{
	}
	ShaderParser::~ShaderParser()
	{
	}
	std::string ShaderParser::spirv_to_glsl(std::vector<uint32_t> spir_v, ShaderStage shader_stage)
	{
		std::string result;



		return std::string();
	}
	std::vector<uint32_t> ShaderParser::glsl_to_spirv(std::string& glsl, ShaderStage shader_stage)
	{
		shaderc::Compiler compiler;

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glsl, convertToShadercFmt(shader_stage, ShaderLang::GLSL), glsl.c_str());

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::string error = result.GetErrorMessage();
			std::cout << error;
			//TRC_ERROR("Error compiling shader\n {}", error.c_str()); --- fix error;
			return std::vector<uint32_t>();
		}

		return { result.cbegin(), result.cend()};
	}
	std::string ShaderParser::load_shader_file(const std::string& filename)
	{
		std::string result;

		FileHandle file_handle;

		if (!FileSystem::open_file(filename, FileMode::READ, file_handle))
		{
			TRC_ERROR("File to open file {}", filename.c_str());
			return result;
		}

		TRC_ASSERT(file_handle.m_isVaild, "invalid file handle please ensure file is vaild");
		FileSystem::read_all_lines(file_handle, result);
		FileSystem::close_file(file_handle);

		return result;
	}
}

shaderc_shader_kind convertToShadercFmt(trace::ShaderStage stage, trace::ShaderLang lang)
{
	switch (stage)
	{
	case trace::ShaderStage::VERTEX_SHADER:
	{
		switch (lang)
		{
		case trace::ShaderLang::GLSL:
		{
			return shaderc_shader_kind::shaderc_glsl_vertex_shader;
		}
		}
	}

	case trace::ShaderStage::PIXEL_SHADER:
	{
		switch (lang)
		{
		case trace::ShaderLang::GLSL:
		{
			return shaderc_shader_kind::shaderc_glsl_fragment_shader;
		}
		}
	}

	}
}
