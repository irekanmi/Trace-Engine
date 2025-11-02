#include "pch.h"

#include "shader_graph/ShaderGraph.h"
#include "shader_graph/ShaderGraphNode.h"
#include "core/io/Logging.h"
#include "render/ShaderParser.h"
#include "resource/DefaultAssetsManager.h"
#include "resource/GenericAssetManager.h"
#include "render/Renderer.h"
#include "backends/Renderutils.h"
#include "core/defines.h"
#include "serialize/GenericSerializer.h"


#include "spdlog/fmt/fmt.h"



namespace trace {


	bool ShaderGraph::Create()
	{
		//TEMP: Find a better way to keep asset loaded
		m_refCount++;
		return true;
	}

	bool ShaderGraph::Create(MaterialType type)
	{
		//TEMP: Find a better way to keep asset loaded
		m_refCount++;
		switch (type)
		{
		case MaterialType::OPAQUE_LIT:
		{

			UUID pbr_node_id = CreateNode<FinalPBRNode>();
			fragment_shader_root = pbr_node_id;

			m_type = type;

			return true;
			break;
		}
		case MaterialType::TRANSPARENT_UNLIT:
		{

			UUID final_node = CreateNode<ConstantNode>();
			ConstantNode* _node = (ConstantNode*)GetNode(final_node);
			_node->SetType(ShaderNodeType::Transparent_UnLit_Node);
			fragment_shader_root = final_node;

			m_type = type;

			return true;
			break;
		}
		case MaterialType::PARTICLE_BILLBOARD:
		{

			UUID final_node = CreateNode<ConstantNode>();
			ConstantNode* _node = (ConstantNode*)GetNode(final_node);
			_node->SetType(ShaderNodeType::Particle_Billboard_Node);
			fragment_shader_root = final_node;
			m_cullMode = CullMode::NONE;

			m_type = type;

			return true;
			break;
		}
		}

		return false;
	}

	void ShaderGraph::Destroy()
	{
		DestroyGraph();
	}

	ShaderGraphNode* ShaderGraph::GetFragmentShaderNode()
	{
		return (ShaderGraphNode*)GetNode(fragment_shader_root);
	}

	Ref<GPipeline> ShaderGraph::GetPipeline()
	{
		std::string pipeline_name = GetName() + RENDER_PIPELINE_FILE_EXTENSION;
		Ref<GPipeline> result = GenericAssetManager::get_instance()->TryGet<GPipeline>(pipeline_name);
		if (result)
		{
			return result;
		}

		Ref<ShaderGraph> graph = GenericAssetManager::get_instance()->Get<ShaderGraph>(m_assetID);
		ShaderGraphInstance instance;
		instance.CreateInstance(graph);
		result = instance.CompileGraph();
		if (result)
		{
			result->SetShaderGraph(this);
			result->SetType(m_type);
		}
		instance.DestroyInstance();

		return result;
	}

	Ref<ShaderGraph> ShaderGraph::Deserialize(UUID id)
	{
		Ref<ShaderGraph> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<ShaderGraph>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<ShaderGraph>(id);
		}

		return result;
	}

	Ref<ShaderGraph> ShaderGraph::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<ShaderGraph>(stream);
	}

	bool ShaderGraphInstance::CreateInstance(Ref<ShaderGraph> shader_graph)
	{
		if (!shader_graph)
		{
			TRC_ERROR("Invalid Shader Graph Handle, {}", __FUNCTION__);
			return false;
		}
		m_shaderGraph = shader_graph;
		bool result = true;

		for (auto& i : m_shaderGraph->GetNodes())
		{
			result = i.second->Instanciate(this) && result;
		}

		return result;
	}

	void ShaderGraphInstance::DestroyInstance()
	{
		final_code.clear();

		for (auto& i : m_nodesData)
		{
			delete i.second;
		}
	}

	void ShaderGraphInstance::WriteNodeCode(std::string& code)
	{
		final_code += "\t" + code + "\n";
	}

	std::string* ShaderGraphInstance::GetParamString(const std::string& param_name)
	{
		auto it = m_paramString.find(param_name);
		if (it != m_paramString.end())
		{
			return &m_paramString[param_name];
		}
		return nullptr;
	}

	Ref<GPipeline> ShaderGraphInstance::CompileGraph()
	{

		std::string shader_struct_variables = "";

		uint32_t vec4_index = 0;
		std::unordered_map<std::string, InternalMaterialData> shader_variables;
		std::unordered_map<std::string, std::string> variable_map;
		for (uint32_t i = 0; i < m_shaderGraph->GetParameters().size(); i++)
		{
			GenericParameter& param = m_shaderGraph->GetParameters()[i];
			if (param.type == GenericValueType::Vec4)
			{
				std::string name_ = "_data_" + std::to_string(vec4_index);
				shader_variables[param.name].hash = 0;
				shader_variables[param.name].internal_data = glm::vec4(0.0f);
				shader_variables[param.name].offset = 0;
				shader_variables[param.name].type = ShaderData::CUSTOM_DATA_VEC4;
				variable_map[param.name] = name_;
				shader_struct_variables += "\t" + std::string("vec4 ") + name_ + ";\n";
				vec4_index++;

				m_paramString[param.name] = name_;

			}
		}

		for (uint32_t i = 0; i < m_shaderGraph->GetParameters().size(); i++)
		{
			GenericParameter& param = m_shaderGraph->GetParameters()[i];
			if (param.type == GenericValueType::Vec3)
			{
				std::string name_ = "_data_" + std::to_string(vec4_index);
				shader_variables[param.name].hash = 0;
				shader_variables[param.name].internal_data = glm::vec3(0.0f);
				shader_variables[param.name].offset = 0;
				shader_variables[param.name].type = ShaderData::CUSTOM_DATA_VEC3;
				variable_map[param.name] = name_;
				shader_struct_variables += "\t" + std::string("vec4 ") + name_ + ";\n";
				vec4_index++;
				m_paramString[param.name] = name_ + ".xyz";
			}
		}


		uint32_t vec2_index = 0;
		for (uint32_t i = 0; i < m_shaderGraph->GetParameters().size(); i++)
		{
			GenericParameter& param = m_shaderGraph->GetParameters()[i];
			if (param.type == GenericValueType::Vec2)
			{
				std::string name_ = "_data_" + std::to_string(vec4_index);
				shader_variables[param.name].hash = 0;
				shader_variables[param.name].internal_data = glm::vec2(0.0f);
				shader_variables[param.name].offset = vec2_index * sizeof(glm::vec2);
				shader_variables[param.name].type = ShaderData::CUSTOM_DATA_VEC2;
				variable_map[param.name] = name_;
				if (vec2_index == 1)
				{
					vec2_index = 0;
					vec4_index++;
					m_paramString[param.name] = name_ + ".zw";
					continue;
				}
				else if (vec2_index == 0)
				{
					shader_struct_variables += "\t" + std::string("vec4 ") + name_ + ";\n";
					vec2_index++;
					m_paramString[param.name] = name_ + ".xy";
				}

			}
		}

		if (vec2_index != 0)
		{
			vec4_index++;
		}

		uint32_t float_index = 0;
		for (uint32_t i = 0; i < m_shaderGraph->GetParameters().size(); i++)
		{
			GenericParameter& param = m_shaderGraph->GetParameters()[i];
			if (param.type == GenericValueType::Float)
			{
				std::string name_ = "_data_" + std::to_string(vec4_index);
				shader_variables[param.name].hash = 0;
				shader_variables[param.name].internal_data = float(0.0f);
				shader_variables[param.name].offset = float_index * sizeof(float);
				shader_variables[param.name].type = ShaderData::CUSTOM_DATA_FLOAT;
				variable_map[param.name] = name_;
				if (float_index == 3)
				{
					float_index = 0;
					vec4_index++;
					m_paramString[param.name] = name_ + ".w";
					continue;
				}
				else if (float_index == 0)
				{
					shader_struct_variables += "\t" + std::string("vec4 ") + name_ + ";\n";
					m_paramString[param.name] = name_ + ".x";
				}
				else if (float_index == 1)
				{
					m_paramString[param.name] = name_ + ".y";
				}
				else if (float_index == 2)
				{
					m_paramString[param.name] = name_ + ".z";
				}
				float_index++;

			}
		}

		if (float_index != 0)
		{
			vec4_index++;
		}

		uint32_t int_index = 0;
		for (uint32_t i = 0; i < m_shaderGraph->GetParameters().size(); i++)
		{
			GenericParameter& param = m_shaderGraph->GetParameters()[i];
			if (param.type == GenericValueType::Int)
			{
				std::string name_ = "_data_" + std::to_string(vec4_index);
				shader_variables[param.name].hash = 0;
				shader_variables[param.name].internal_data = int(0);
				shader_variables[param.name].offset = int_index * sizeof(int);
				shader_variables[param.name].type = ShaderData::CUSTOM_DATA_INT;
				variable_map[param.name] = name_;
				if (int_index == 3)
				{
					int_index = 0;
					vec4_index++;
					m_paramString[param.name] = name_ + ".w";
					continue;
				}
				else if (int_index == 0)
				{
					shader_struct_variables += "\t" + std::string("ivec4 ") + name_ + ";\n";
					m_paramString[param.name] = name_ + ".x";
				}
				else if (int_index == 1)
				{
					m_paramString[param.name] = name_ + ".y";
				}
				else if (int_index == 2)
				{
					m_paramString[param.name] = name_ + ".z";
				}
				int_index++;

			}
		}

		std::string texture_inputs;
		int32_t texture_index = 0;

		for (uint32_t i = 0; i < m_shaderGraph->GetParameters().size(); i++)
		{
			GenericParameter& param = m_shaderGraph->GetParameters()[i];
			if (param.type == GenericValueType::Sampler2D)
			{
				std::string text = fmt::format("\tINSTANCE_TEXTURE_INDEX({}, {});\n", param.name, texture_index);
				texture_inputs += text;
				m_paramString[param.name] = fmt::format("GET_BINDLESS_TEXTURE2D({})", param.name);
				texture_index++;

			}
		}

		m_shaderGraph->GetFragmentShaderNode()->Run(this);

		switch (m_shaderGraph->GetType())
		{
		case MaterialType::OPAQUE_LIT:
		{	

			

			std::string fragment_shader_code = R"(
#version 450


#include "globals_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"
#include "functions.glsl"

OUT_FRAG_DATA
IN_VERTEX_DATA



struct InstanceBufferObject
{{
    {}
}};

layout(std140, set = 1, binding = 3) readonly buffer MaterialData{{
    InstanceBufferObject objects[];
}};

BINDLESS_COMBINED_SAMPLER2D;

void main()
{{
	{}
	vec3 view_direction = normalize(-_fragPos);


	{}
}}
			
			)";

			bool has_data = !shader_variables.empty();
			bool has_texture = !texture_inputs.empty();

			fragment_shader_code = fmt::format(fragment_shader_code, has_data ? shader_struct_variables : "vec4 _data_0;", has_texture ? texture_inputs : " ", final_code);

			FileHandle file;
			FileSystem::open_file("C:/Dev/Trace_Projects/Project_TD_t0/Debug/shader_graph_debug.glsl", FileMode::WRITE, file);
			FileSystem::writestring(file, fragment_shader_code);
			FileSystem::close_file(file);

			std::vector<std::pair<std::string, int>> data_index;
			std::vector<uint32_t> spriv_code = ShaderParser::glsl_to_spirv(fragment_shader_code, ShaderStage::PIXEL_SHADER, data_index);

			if (spriv_code.empty())
			{
				return Ref<GPipeline>();
			}

			std::string& asset_name = m_shaderGraph->GetName();
			std::string shader_name = asset_name + ".frag";

			GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

			Ref<GShader> frag_shader = asset_manager->Get<GShader>(shader_name);
			if (frag_shader)
			{
				frag_shader->m_refCount = 1;
				frag_shader.free();
			}

			frag_shader = asset_manager->CreateAssetHandle_<GShader>(shader_name, spriv_code, data_index, ShaderStage::PIXEL_SHADER);

			if (!frag_shader)
			{
				return Ref<GPipeline>();
			}

			Ref<GShader> vert_shader = asset_manager->Get<GShader>("trace_core.shader.vert.glsl");


			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(vert_shader.get(), s_res);
			ShaderParser::generate_shader_resources(frag_shader.get(), s_res);

			PipelineStateDesc _ds;
			_ds.vertex_shader = vert_shader.get();
			_ds.pixel_shader = frag_shader.get();
			_ds.resources = s_res;

			AutoFillPipelineDesc(
				_ds
			);
			_ds.render_pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");
			_ds.blend_state.alpha_to_blend_coverage = false;
			_ds.rasteriser_state = { m_shaderGraph->GetCullMode(), FillMode::SOLID };

			std::string pipeline_name = asset_name + RENDER_PIPELINE_FILE_EXTENSION;

			Ref<GPipeline> pipeline = asset_manager->Get<GPipeline>(pipeline_name);
			if (pipeline)
			{
				pipeline->RecreatePipeline(_ds);
			}
			else
			{
				pipeline = asset_manager->CreateAssetHandle<GPipeline>(pipeline_name, _ds);

			}

			if (pipeline)
			{
				for (auto& i : variable_map)
				{
					uint32_t hash = pipeline->GetHashTable().Get(i.second);
					shader_variables[i.first].hash = hash;
				}

				MaterialData& pipeline_var = pipeline->GetShaderGraphVariables();
				pipeline_var = shader_variables;
			}

			return pipeline;

			break;
		}
		case MaterialType::TRANSPARENT_UNLIT:
		{	

			

			std::string fragment_shader_code = R"(
#version 450


#include "OIT_data.glsl"
#include "globals_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"
#include "functions.glsl"

OUT_OIT_DATA
IN_VERTEX_DATA

layout(set = 0, binding = 4)uniform sampler2D _screen_color;

struct InstanceBufferObject
{{
    {}
}};

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{{
    mat4 _projection;
    mat4 _view;
	vec4 _time_values;
    vec3 _view_position;
}};

layout(std140, set = 1, binding = 3) readonly buffer MaterialData{{
    InstanceBufferObject objects[];
}};

BINDLESS_COMBINED_SAMPLER2D;

void main()
{{
	{}
	vec3 view_direction = normalize(-_fragPos);
	vec3 ndc = (_clip_space_pos.xyz / _clip_space_pos.w) * 0.5f + 0.5f;
    vec2 screen_uv = ndc.xy;


	{}
}}
			
			)";

			bool has_data = !shader_variables.empty();
			bool has_texture = !texture_inputs.empty();

			fragment_shader_code = fmt::format(fragment_shader_code, has_data ? shader_struct_variables : "vec4 _data_0;", has_texture ? texture_inputs : " ", final_code);

			FileHandle file;
			FileSystem::open_file("C:/Dev/Trace_Projects/Project_TD_t0/Debug/shader_graph_debug.glsl", FileMode::WRITE, file);
			FileSystem::writestring(file, fragment_shader_code);
			FileSystem::close_file(file);

			std::vector<std::pair<std::string, int>> data_index;
			std::vector<uint32_t> spriv_code = ShaderParser::glsl_to_spirv(fragment_shader_code, ShaderStage::PIXEL_SHADER, data_index);

			if (spriv_code.empty())
			{
				return Ref<GPipeline>();
			}

			std::string& asset_name = m_shaderGraph->GetName();
			std::string shader_name = asset_name + ".frag";

			GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

			Ref<GShader> frag_shader = asset_manager->Get<GShader>(shader_name);
			if (frag_shader)
			{
				frag_shader->m_refCount = 1;
				frag_shader.free();
			}

			frag_shader = asset_manager->CreateAssetHandle_<GShader>(shader_name, spriv_code, data_index, ShaderStage::PIXEL_SHADER);

			if (!frag_shader)
			{
				return Ref<GPipeline>();
			}

			Ref<GShader> vert_shader = asset_manager->Get<GShader>("trace_core.shader.vert.glsl");


			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(vert_shader.get(), s_res);
			ShaderParser::generate_shader_resources(frag_shader.get(), s_res);

			PipelineStateDesc _ds;
			_ds.vertex_shader = vert_shader.get();
			_ds.pixel_shader = frag_shader.get();
			_ds.resources = s_res;

			AutoFillPipelineDesc(
				_ds,
				false
			);
			_ds.input_layout = Vertex::get_input_layout();
			_ds.render_pass = Renderer::get_instance()->GetRenderPass("WOIT_PASS");
			Enable_WeightedOIT(_ds);
			_ds.depth_sten_state = { true, false, false, 1.0f, 0.0f };
			_ds.rasteriser_state = { m_shaderGraph->GetCullMode(), FillMode::SOLID};

			std::string pipeline_name = asset_name + RENDER_PIPELINE_FILE_EXTENSION;

			Ref<GPipeline> pipeline = asset_manager->Get<GPipeline>(pipeline_name);
			if (pipeline)
			{
				pipeline->RecreatePipeline(_ds);
			}
			else
			{
				pipeline = asset_manager->CreateAssetHandle<GPipeline>(pipeline_name, _ds);

			}

			if (pipeline)
			{
				for (auto& i : variable_map)
				{
					uint32_t hash = pipeline->GetHashTable().Get(i.second);
					shader_variables[i.first].hash = hash;
				}

				MaterialData& pipeline_var = pipeline->GetShaderGraphVariables();
				pipeline_var = shader_variables;
			}

			return pipeline;

			break;
		}
		case MaterialType::PARTICLE_BILLBOARD:
		{	

			

			std::string fragment_shader_code = R"(
#version 450


#include "OIT_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"
#include "functions.glsl"


OUT_OIT_DATA




layout(location = 0)in vec2 _texCoord;

BINDLESS_COMBINED_SAMPLER2D;


layout(location = 2) in Data{{
    vec3 position;
    vec3 color;
    vec3 scale;
	float lifetime;
}};

struct InstanceBufferObject
{{
    {}
}};

layout(std140, set = 1, binding = 3) readonly buffer MaterialData{{
    InstanceBufferObject objects[];
}};

void main()
{{
	{}
	


	{}
}}
			
			)";

			bool has_data = !shader_variables.empty();
			bool has_texture = !texture_inputs.empty();

			fragment_shader_code = fmt::format(fragment_shader_code, has_data ? shader_struct_variables : "vec4 _data_0;", has_texture ? texture_inputs : " ", final_code);

			FileHandle file;
			FileSystem::open_file("C:/Dev/Trace_Projects/Project_TD_t0/Debug/shader_graph_debug.glsl", FileMode::WRITE, file);
			FileSystem::writestring(file, fragment_shader_code);
			FileSystem::close_file(file);

			std::vector<std::pair<std::string, int>> data_index;
			std::vector<uint32_t> spriv_code = ShaderParser::glsl_to_spirv(fragment_shader_code, ShaderStage::PIXEL_SHADER, data_index);

			if (spriv_code.empty())
			{
				return Ref<GPipeline>();
			}

			std::string& asset_name = m_shaderGraph->GetName();
			std::string shader_name = asset_name + ".frag";

			GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

			Ref<GShader> frag_shader = asset_manager->Get<GShader>(shader_name);
			if (frag_shader)
			{
				frag_shader->m_refCount = 1;
				frag_shader.free();
			}

			frag_shader = asset_manager->CreateAssetHandle_<GShader>(shader_name, spriv_code, data_index, ShaderStage::PIXEL_SHADER);

			if (!frag_shader)
			{
				return Ref<GPipeline>();
			}

			Ref<GShader> vert_shader = asset_manager->Get<GShader>("particle_billboard.vert.glsl");


			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(vert_shader.get(), s_res);
			ShaderParser::generate_shader_resources(frag_shader.get(), s_res);

			PipelineStateDesc _ds;
			_ds.vertex_shader = vert_shader.get();
			_ds.pixel_shader = frag_shader.get();
			_ds.resources = s_res;

			AutoFillPipelineDesc(
				_ds,
				false
			);
			_ds.input_layout = Vertex::get_input_layout();
			_ds.render_pass = Renderer::get_instance()->GetRenderPass("WOIT_PASS");
			Enable_WeightedOIT(_ds);
			_ds.depth_sten_state = { true, false, false, 1.0f, 0.0f };
			_ds.rasteriser_state = { m_shaderGraph->GetCullMode(), FillMode::SOLID};

			std::string pipeline_name = asset_name + RENDER_PIPELINE_FILE_EXTENSION;

			Ref<GPipeline> pipeline = asset_manager->Get<GPipeline>(pipeline_name);
			if (pipeline)
			{
				pipeline->RecreatePipeline(_ds);
			}
			else
			{
				pipeline = asset_manager->CreateAssetHandle<GPipeline>(pipeline_name, _ds);

			}

			if (pipeline)
			{
				for (auto& i : variable_map)
				{
					uint32_t hash = pipeline->GetHashTable().Get(i.second);
					shader_variables[i.first].hash = hash;
				}

				MaterialData& pipeline_var = pipeline->GetShaderGraphVariables();
				pipeline_var = shader_variables;
			}

			return pipeline;

			break;
		}
		}

		return Ref<GPipeline>();
	}

	void ShaderGraphInstance::set_parameter_data(const std::string& name, void* data, uint32_t size)
	{
	}

}