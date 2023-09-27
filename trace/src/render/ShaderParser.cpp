#include "pch.h"

#include "ShaderParser.h"
#include "shaderc/shaderc.hpp"
#include "core/io/Logging.h"
#include "core/FileSystem.h"
#include "spirv_cross/spirv_reflect.h"
#include "render/GShader.h"
#include "resource/ShaderManager.h"
#include <filesystem>

shaderc_shader_kind convertToShadercFmt(trace::ShaderStage stage, trace::ShaderLang lang);

// Interface that determines how include files in shader will be processed
class IncludeInterface : public shaderc::CompileOptions::IncluderInterface
{
public:
	// Handles shaderc_include_resolver_fn callbacks.
	virtual shaderc_include_result* GetInclude(const char* requested_source,
		shaderc_include_type type,
		const char* requesting_source,
		size_t include_depth) override
	{
		std::string shader_res_path = trace::ShaderManager::get_instance()->GetShaderResourcePath();
		std::filesystem::path path(shader_res_path);
		path /= requested_source;

		IncludeInterface::include_src_name = path.string();
		IncludeInterface::include_src_data = trace::ShaderParser::load_shader_file(IncludeInterface::include_src_name);

		shaderc_include_result* result = new shaderc_include_result();
		result->content = IncludeInterface::include_src_data.c_str();
		result->content_length = IncludeInterface::include_src_data.length();
		result->source_name = IncludeInterface::include_src_name.c_str();
		result->source_name_length = IncludeInterface::include_src_name.length();
		result->user_data = nullptr;

		return result;
	}

	// Handles shaderc_include_result_release_fn callbacks.
	virtual void ReleaseInclude(shaderc_include_result* data) override
	{
		IncludeInterface::include_src_data.clear();
		IncludeInterface::include_src_name.clear();
		delete data;
	}

	static std::string include_src_data;
	static std::string include_src_name;
};

std::string IncludeInterface::include_src_data = std::string();
std::string IncludeInterface::include_src_name = std::string();

namespace trace {
	static void GenShaderRes(const std::vector<uint32_t>& code, ShaderResources& out_res, ShaderStage shader_stage)
	{
		SpvReflectShaderModule shader;
		SpvReflectResult result = spvReflectCreateShaderModule(code.size() * 4, code.data(), &shader);
		TRC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "failed to compile shader, {}", __FUNCTION__);

		for (uint32_t i = 0; i < shader.descriptor_set_count; i++)
		{
			SpvReflectDescriptorSet& set = shader.descriptor_sets[i];
			ShaderResourceStage res_stage = ShaderResourceStage::RESOURCE_STAGE_NONE;
			if (set.set == 0) res_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
			else if (set.set == 1) res_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;

			for (uint32_t j = 0; j < set.binding_count; j++)
			{
				SpvReflectDescriptorBinding*& binding = set.bindings[j];
				ShaderArray shArray;
				ShaderArray::ArrayInfo arr_info;
				ShaderStruct shStruct;
				ShaderStruct::StructInfo stu_info;
				bool isArray = (binding->count > 1);
				bool u_buffer = (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				bool u_cSampler = (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

				if (u_buffer && isArray)
				{
					SpvReflectBlockVariable& block = binding->block;
					ShaderArray Array;
					Array.count = binding->count;
					Array.name = block.name;
					Array.resource_size = block.padded_size;
					Array.resource_stage = res_stage;
					Array.shader_stage = shader_stage;
					Array.slot = binding->binding;
					Array.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
					out_res.resources.push_back({ {}, Array, {}, ShaderDataDef::STRUCT_ARRAY });
				}
				else if (u_buffer && !isArray)
				{
					SpvReflectBlockVariable& block = binding->block;
					ShaderStruct Struct;
					Struct.resource_size = block.padded_size;
					Struct.resource_stage = res_stage;
					Struct.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
					Struct.shader_stage = shader_stage;
					Struct.slot = binding->binding;

					for (uint32_t k = 0; k < block.member_count; k++)
					{
						SpvReflectBlockVariable& blck_var = block.members[k];
						ShaderStruct::StructInfo s_info;
						s_info.resource_name = blck_var.name;
						s_info.resource_size = blck_var.size;
						Struct.members.push_back(s_info);
					}
					out_res.resources.push_back({ Struct, {}, {}, ShaderDataDef::STRUCTURE });
				}

				if (u_cSampler)
				{
					SpvReflectImageTraits& tex = binding->image;
					ShaderArray Array;
					Array.count = binding->count;
					Array.name = binding->name;
					Array.resource_stage = res_stage;
					Array.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
					Array.shader_stage = shader_stage;
					Array.slot = binding->binding;

					if (!isArray)
					{
						Array.count = 1;
						ShaderArray::ArrayInfo a_info;
						a_info.index = 0;
						a_info.resource_name = binding->name;
						Array.members.push_back(a_info);
					}
					else
					{
						for (uint32_t k = 0; k < binding->count; k++)
						{
							ShaderArray::ArrayInfo a_info;
							a_info.index = k;
							a_info.resource_name = std::string(binding->name) + std::to_string(k);
							Array.members.push_back(a_info);
						}
					}
					
					
					out_res.resources.push_back({ {}, Array, {}, ShaderDataDef::ARRAY });

				}
			}
		}

		for (uint32_t i = 0; i < shader.push_constant_block_count; i++)
		{
			SpvReflectBlockVariable& block = shader.push_constant_blocks[i];
			ShaderStruct Struct;
			Struct.resource_size = block.padded_size;
			Struct.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
			Struct.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
			Struct.shader_stage = shader_stage;

			for (uint32_t k = 0; k < block.member_count; k++)
			{
				SpvReflectBlockVariable& blck_var = block.members[k];
				ShaderStruct::StructInfo s_info;
				s_info.resource_name = blck_var.name;
				s_info.resource_size = blck_var.size;
				Struct.members.push_back(s_info);
			}
			out_res.resources.push_back({ Struct, {}, {}, ShaderDataDef::STRUCTURE });
		}

		return;
	}

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
	std::vector<uint32_t> ShaderParser::glsl_to_spirv(const std::string& glsl, ShaderStage shader_stage)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions opt;
		opt.SetIncluder(std::make_unique<IncludeInterface>());
		opt.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

		// TODO: Fix and check why CompileGlslToSpv() requires "const char* input file name"                                        "PlaceHolder"
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glsl, convertToShadercFmt(shader_stage, ShaderLang::GLSL), "Trace_shader", opt);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::string error = result.GetErrorMessage();
			TRC_ERROR("Error compiling shader\n {}", error); ;
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
	void ShaderParser::generate_shader_resources(const std::string& shader_src, ShaderResources& out_res, ShaderStage shader_stage)
	{
		std::vector<uint32_t> code = glsl_to_spirv(shader_src, shader_stage);

		GenShaderRes(code, out_res, shader_stage);
	}
	void ShaderParser::generate_shader_resources(GShader* shader, ShaderResources& out_res)
	{
		std::vector<uint32_t>& code = shader->GetCode();
		GenShaderRes(code, out_res, shader->GetShaderStage());
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
