#include "pch.h"

#include "ShaderParser.h"
#include "shaderc/shaderc.hpp"
#include "core/io/Logging.h"
#include "core/FileSystem.h"
#include "spirv_cross/spirv_reflect.h"
#include "render/GShader.h"

#include "scene/UUID.h"
#include "spdlog/fmt/fmt.h"
#include "core/Utils.h"
#include "external_utils.h"
#include "resource/DefaultAssetsManager.h"

#include <filesystem>

shaderc_shader_kind convertToShadercFmt(trace::ShaderStage stage, trace::ShaderLang lang);

std::unordered_map<std::string, std::string> include_files;

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
		std::string shader_res_path = trace::DefaultAssetsManager::assets_path + "/shaders";
		std::filesystem::path path(shader_res_path);
		path /= requested_source;

		/*IncludeInterface::include_src_name = path.string();
		IncludeInterface::include_src_data = trace::ShaderParser::load_shader_file(IncludeInterface::include_src_name);*/

		std::string file_data = trace::ShaderParser::load_shader_file(path.string());
		std::string file_name = requested_source;
		include_files[file_name] = file_data;

		shaderc_include_result* result = new shaderc_include_result();
		result->content = include_files[file_name].c_str();
		result->content_length = include_files[file_name].length();
		result->source_name = include_files.find(file_name)->first.c_str();
		result->source_name_length = include_files.find(file_name)->first.length();
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

	
	static ShaderData GetDataType(SpvReflectTypeDescription& desc)
	{
		ShaderData result = ShaderData::NONE;

		bool _float = (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT));
		bool _int = (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT));
		bool _vec = (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_VECTOR));
		bool _mat = (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_MATRIX));
		bool _tex = (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE)) || (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE));

		if (_float && !_vec && !_mat)
		{
			result = ShaderData::CUSTOM_DATA_FLOAT;
		}

		if (_int && !_vec && !_mat)
		{
			result = ShaderData::CUSTOM_DATA_INT;
		}

		if (_float && _vec && !_mat)
		{
			if(desc.traits.numeric.vector.component_count == 4)
				result = ShaderData::CUSTOM_DATA_VEC4;
			if (desc.traits.numeric.vector.component_count == 3)
				result = ShaderData::CUSTOM_DATA_VEC3;
			if (desc.traits.numeric.vector.component_count == 2)
				result = ShaderData::CUSTOM_DATA_VEC2;
		}

		if (_int && _vec && !_mat)
		{
			if (desc.traits.numeric.vector.component_count == 4)
				result = ShaderData::CUSTOM_DATA_IVEC4;
			if (desc.traits.numeric.vector.component_count == 3)
				result = ShaderData::CUSTOM_DATA_IVEC3;
			if (desc.traits.numeric.vector.component_count == 2)
				result = ShaderData::CUSTOM_DATA_IVEC2;
		}

		if (_float && _mat)
		{
			if (desc.traits.numeric.matrix.row_count == 4 && desc.traits.numeric.matrix.column_count == 4)
				result = ShaderData::CUSTOM_DATA_MAT4;
			if (desc.traits.numeric.matrix.row_count == 3 && desc.traits.numeric.matrix.column_count == 3)
				result = ShaderData::CUSTOM_DATA_MAT3;
		}

		if (_tex)
		{
			result = ShaderData::CUSTOM_DATA_TEXTURE;
		}

		if (TRC_HAS_FLAG(desc.type_flags, SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT))
		{
			result = ShaderData::CUSTOM_DATA_BOOL;
		}

		return result;
	}

	static ShaderResourceType GetResourceType(SpvReflectDescriptorBinding* binding)
	{

		switch (binding->descriptor_type)
		{
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		{
			return ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		}
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		{
			return ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		}
		case SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			return ShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER;
		}
		}

		return ShaderResourceType::SHADER_RESOURCE_TYPE_NOUSE;
	}

	static void GetBlockVariables(SpvReflectBlockVariable& block, ShaderResource& resource)
	{
		for (uint32_t k = 0; k < block.member_count; k++)
		{
			SpvReflectBlockVariable& blck_var = block.members[k];
			ShaderResource::Member member_info;
			member_info.resource_name = blck_var.name;
			member_info.resource_size = blck_var.size;
			member_info.offset = blck_var.offset;
			member_info.resource_data_type = GetDataType(*blck_var.type_description);
			resource.members.push_back(member_info);
		}
	}

	static void GenShaderRes(const std::vector<uint32_t>& code, ShaderResources& out_res, ShaderStage shader_stage)
	{
		SpvReflectShaderModule shader;
		SpvReflectResult result = spvReflectCreateShaderModule(code.size() * 4, code.data(), &shader);
		TRC_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "failed to compile shader, {}", __FUNCTION__);

		std::unordered_map<std::string, int> push_map;

		for (uint32_t i = 0; i < shader.descriptor_set_count; i++)
		{
			SpvReflectDescriptorSet& set = shader.descriptor_sets[i];
			ShaderResourceStage res_stage = ShaderResourceStage::RESOURCE_STAGE_NONE;
			if (set.set == 0)
			{
				res_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
			}
			else if (set.set == 1)
			{
				res_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
			}

			bool is_instance = (set.set == 1);

			for (uint32_t j = 0; j < set.binding_count; j++)
			{
				SpvReflectDescriptorBinding* binding = set.bindings[j];

				bool is_storage_buffer = (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER);

				// Checking if a particular binding is already in the resoures
				bool skip = false;
				for (auto& res : out_res.resources)
				{					
					skip = (res.resource_stage == res_stage) && (res.slot == binding->binding);
					if (skip)
					{
						uint32_t _stage = res.shader_stage | shader_stage;
						res.shader_stage = (ShaderStage)_stage;
						break;
					}
				}

				if (skip)
				{
					continue;
				}

				ShaderResource resource;

				resource.resource_name = binding->name;
				resource.count = binding->count > 0 ? binding->count : 1;
				resource.resource_stage = res_stage;
				resource.shader_stage = shader_stage;
				resource.slot = binding->binding;
				resource.resource_type = GetResourceType(binding);
				
				bool is_structure = (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) || (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) || (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) || (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
				bool is_image = (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) || (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (binding->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE);

				if (is_structure)
				{

					SpvReflectBlockVariable& block = binding->block;

					resource.def = ShaderDataDef::STRUCTURE;
					if (is_instance && is_storage_buffer)
					{
						SpvReflectBlockVariable& block_var = block.members[0];
						resource.resource_size = block_var.padded_size;
						GetBlockVariables(block_var, resource);
					}
					else
					{
						resource.resource_size = block.padded_size;
						GetBlockVariables(block, resource);
					}
					

					out_res.resources.push_back(resource);
				}
				else if (is_image)
				{
					SpvReflectImageTraits& tex = binding->image;

					resource.def = ShaderDataDef::IMAGE;

					out_res.resources.push_back(resource);
				}

			}
		}

		int index = 0;
		for (auto& i : out_res.resources)
		{
			push_map[i.resource_name] = index;
			index++;
		}

		for (uint32_t i = 0; i < shader.push_constant_block_count; i++)
		{
			SpvReflectBlockVariable& block = shader.push_constant_blocks[i];

			auto it = push_map.find(block.name);
			if (it != push_map.end())
			{
				uint32_t _stage = out_res.resources[it->second].shader_stage | shader_stage;
				out_res.resources[it->second].shader_stage = (ShaderStage)_stage;
				continue;
			}

			ShaderResource resource;

			resource.resource_name = block.name;
			resource.count = 1;
			resource.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
			resource.shader_stage = shader_stage;
			resource.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;

			resource.def = ShaderDataDef::STRUCTURE;
			resource.resource_size = block.padded_size;
			for (uint32_t k = 0; k < block.member_count; k++)
			{
				SpvReflectBlockVariable& blck_var = block.members[k];
				ShaderResource::Member member_info;
				member_info.resource_name = blck_var.name;
				member_info.resource_size = blck_var.size;
				member_info.offset = blck_var.offset;
				member_info.resource_data_type = GetDataType(*blck_var.type_description);
				resource.members.push_back(member_info);
			}

			out_res.resources.push_back(resource);

			
		}

		return;
	}
	static void GenShaderDataIndex(const std::string& code, std::vector<std::pair<std::string, int>>& out_index, ShaderStage shader_stage)
	{
		

		std::vector<std::string> data = SplitString(code, '\n');

		for (auto& i : data)
		{
			if (i.find("TexIndex ") != std::string::npos)
			{
				std::vector<std::string> index_line = SplitString(i, ' ');
				int32_t value_index = static_cast<int32_t>(index_line.size()) - 9;
				int32_t name_index = value_index - 3;

				int32_t slot_value_index = static_cast<int32_t>(index_line.size()) - 4;
				

				std::string& name = index_line[name_index];
				int32_t index_value = std::stoi(index_line[value_index]);

				int32_t slot_value = std::stoi(index_line[slot_value_index]);

				int32_t value = (slot_value << 16) | (index_value);

				out_index.push_back(std::make_pair(name, value));
			}
		}

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
	std::vector<uint32_t> ShaderParser::glsl_to_spirv(const std::string& glsl, ShaderStage shader_stage, std::vector<std::pair<std::string, int>>& out_data_index)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions opt;
		opt.SetIncluder(std::make_unique<IncludeInterface>());
		opt.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		// TODO: Fix and check why CompileGlslToSpv() requires "const char* input file name"                                        "PlaceHolder"
		shaderc::PreprocessedSourceCompilationResult pre_result = compiler.PreprocessGlsl(glsl, convertToShadercFmt(shader_stage, ShaderLang::GLSL), "Trace_shader", opt);
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glsl, convertToShadercFmt(shader_stage, ShaderLang::GLSL), "Trace_shader", opt);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::string error = result.GetErrorMessage();
			TRC_ERROR("Error compiling shader\n {}", error);
			return std::vector<uint32_t>();
		}

		if (pre_result.GetCompilationStatus() == shaderc_compilation_status_success)
		{
			std::string data = pre_result.cbegin();
			TRC_INFO("Preprocess Shader: {}", data);
			GenShaderDataIndex(data, out_data_index, shader_stage);
		}
		else
		{
			std::string error = pre_result.GetErrorMessage();
			TRC_ERROR("Error compiling shader\n {}", error);
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
		std::vector<std::pair<std::string, int>> data_index;
		std::vector<uint32_t> code = glsl_to_spirv(shader_src, shader_stage, data_index);

		GenShaderRes(code, out_res, shader_stage);
	}
	void ShaderParser::generate_shader_resources(GShader* shader, ShaderResources& out_res)
	{
		std::vector<uint32_t>& code = shader->GetCode();
		GenShaderRes(code, out_res, shader->GetShaderStage());
	}
	void ShaderParser::generate_shader_data_index(GShader* shader, std::vector<std::pair<std::string, int>>& out_index)
	{
		std::vector<uint32_t>& code = shader->GetCode();
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

	return shaderc_shader_kind::shaderc_glsl_infer_from_source;
}
