#include "pch.h"

#include "PipelineManager.h"
#include "core/Coretypes.h"
#include "render/Renderer.h"
#include "core/FileSystem.h"
#include "render/GShader.h"
#include "render/ShaderParser.h"
#include "render/Renderutils.h"

namespace trace {

	PipelineManager* PipelineManager::s_instance = nullptr;

	PipelineManager::PipelineManager()
	{
	}
	PipelineManager::PipelineManager(uint32_t max_entires)
	{
	}
	PipelineManager::~PipelineManager()
	{
	}
	bool PipelineManager::Init(uint32_t max_entries)
	{
		m_numEntries = max_entries;
		m_hashtable.Init(m_numEntries);
		m_hashtable.Fill(INVALID_ID);

		m_pipelines.resize(m_numEntries);

		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_pipelines[i].m_id = INVALID_ID;
		}

		return true;
	}
	void PipelineManager::ShutDown()
	{
		standard_pipeline.~Ref();
		skybox_pipeline.~Ref();
		if (!m_pipelines.empty())
		{
			
			for (GPipeline& pipeline : m_pipelines)
			{
				if (pipeline.m_id == INVALID_ID)
					continue;
				pipeline.~GPipeline();
			}
			m_pipelines.clear();
		}

	}
	bool PipelineManager::CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill)
	{

		if (auto_fill || desc.render_pass == nullptr)
		{
			desc.render_pass = Renderer::get_instance()->GetRenderPass(desc._renderPass);
		}


		bool found = false;
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			if (m_pipelines[i].m_id == INVALID_ID)
			{
				RenderFunc::CreatePipeline(&m_pipelines[i], desc);
				if (!RenderFunc::InitializePipeline(&m_pipelines[i]))
					return false;
				m_pipelines[i].m_id = i;
				m_hashtable.Set(name, i);
				found = true;
				break;
			}
		}

		if (!found)
		{
			TRC_ERROR("Failed to create pipeline {}", name);
			return false;
		}

		return true;
	}
	GPipeline* PipelineManager::GetPipeline(const std::string& name)
	{
		uint32_t hash = m_hashtable.Get(name);

		if (hash != INVALID_ID)
		{
			return &m_pipelines[hash];
		}

		return nullptr;
	}
	Ref<GPipeline> PipelineManager::GetDefault(const std::string& name)
	{
		if (name == "standard")
		{
			return standard_pipeline;
		}
		if (name == "skybox")
		{
			return skybox_pipeline;
		}

		return Ref<GPipeline>();
	}
	void PipelineManager::Unload(GPipeline* pipeline)
	{
		if (pipeline->m_refCount > 0)
		{
			TRC_WARN("Can't unload a pipeline that is still in use");
			return;
		}

		pipeline->m_id = INVALID_ID;
		RenderFunc::DestroyPipeline(pipeline);
		pipeline->~GPipeline();

	}
	bool PipelineManager::LoadDefaults()
	{


		std::string vert_src;
		std::string frag_src;

		vert_src = ShaderParser::load_shader_file("../assets/shaders/trace_core.shader.vert.glsl");
		frag_src = ShaderParser::load_shader_file("../assets/shaders/trace_core.shader.frag.glsl");

		std::cout << vert_src;
		std::cout << frag_src;

		GShader VertShader;
		GShader FragShader;

		RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
		RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

		ShaderResourceBinding projection;
		projection.shader_stage = ShaderStage::VERTEX_SHADER;
		projection.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		projection.resource_size = sizeof(glm::mat4);
		projection.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		projection.resource_name = "projection";
		projection.count = 1;
		projection.index = 0;
		projection.slot = 0;
		projection.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		ShaderResourceBinding view;
		view.shader_stage = ShaderStage::VERTEX_SHADER;
		view.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		view.resource_size = sizeof(glm::mat4);
		view.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		view.resource_name = "view";
		view.count = 1;
		view.index = 0;
		view.slot = 0;
		view.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		ShaderResourceBinding view_position;
		view_position.shader_stage = ShaderStage::VERTEX_SHADER;
		view_position.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		view_position.resource_size = sizeof(glm::vec3);
		view_position.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		view_position.resource_name = "view_position";
		view_position.count = 1;
		view_position.index = 0;
		view_position.slot = 0;
		view_position.resource_data_type = ShaderData::CUSTOM_DATA_VEC3;

		ShaderResourceBinding _test;
		_test.shader_stage = ShaderStage::VERTEX_SHADER;
		_test.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		_test.resource_size = sizeof(glm::vec2);
		_test.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		_test.resource_name = "_test";
		_test.count = 1;
		_test.index = 0;
		_test.slot = 0;
		_test.resource_data_type = ShaderData::CUSTOM_DATA_VEC2;


		ShaderResourceBinding diffuse_map;
		diffuse_map.shader_stage = ShaderStage::PIXEL_SHADER;
		diffuse_map.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		diffuse_map.resource_size = 0;
		diffuse_map.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		diffuse_map.resource_name = "diffuse_map";
		diffuse_map.count = 3;
		diffuse_map.index = 0;
		diffuse_map.slot = 1;
		diffuse_map.resource_data_type = ShaderData::MATERIAL_ALBEDO;

		ShaderResourceBinding specular_map;
		specular_map.shader_stage = ShaderStage::PIXEL_SHADER;
		specular_map.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		specular_map.resource_size = 0;
		specular_map.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		specular_map.resource_name = "specular_map";
		specular_map.count = 3;
		specular_map.index = 1;
		specular_map.slot = 1;
		specular_map.resource_data_type = ShaderData::MATERIAL_SPECULAR;

		ShaderResourceBinding normal_map;
		normal_map.shader_stage = ShaderStage::PIXEL_SHADER;
		normal_map.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		normal_map.resource_size = 0;
		normal_map.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		normal_map.resource_name = "normal_map";
		normal_map.count = 3;
		normal_map.index = 2;
		normal_map.slot = 1;
		normal_map.resource_data_type = ShaderData::MATERIAL_NORMAL;

		

		ShaderResourceBinding diffuse_color;
		diffuse_color.shader_stage = ShaderStage::PIXEL_SHADER;
		diffuse_color.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		diffuse_color.resource_size = sizeof(glm::vec4);
		diffuse_color.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		diffuse_color.resource_name = "diffuse_color";
		diffuse_color.count = 1;
		diffuse_color.index = 0;
		diffuse_color.slot = 0;
		diffuse_color.resource_data_type = ShaderData::MATERIAL_DIFFUSE_COLOR;

		ShaderResourceBinding shininess;
		shininess.shader_stage = ShaderStage::PIXEL_SHADER;
		shininess.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		shininess.resource_size = sizeof(float);
		shininess.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		shininess.resource_name = "shininess";
		shininess.count = 1;
		shininess.index = 0;
		shininess.slot = 0;
		shininess.resource_data_type = ShaderData::MATERIAL_SHININESS;

		ShaderResourceBinding rest;
		rest.shader_stage = ShaderStage::PIXEL_SHADER;
		rest.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		rest.resource_size = sizeof(glm::ivec4);
		rest.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		rest.resource_name = "rest";
		rest.count = 1;
		rest.index = 0;
		rest.slot = 2;
		rest.resource_data_type = ShaderData::CUSTOM_DATA_VEC4;

		

		ShaderResourceBinding model;
		model.shader_stage = ShaderStage::VERTEX_SHADER;
		model.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
		model.resource_size = sizeof(glm::mat4);
		model.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		model.resource_name = "model";
		model.count = 1;
		model.index = 0;
		model.slot = 3;
		model.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;

		std::vector<ShaderResourceBinding> scene = {
			projection,
			view,
			view_position,
			_test,
			diffuse_map,
			specular_map,
			normal_map,
			diffuse_color,
			shininess,
			model,
			rest
		};

		PipelineStateDesc _ds;
		_ds.resource_bindings_count = 11;
		_ds.resource_bindings = scene;
		_ds.vertex_shader = &VertShader;
		_ds.pixel_shader = &FragShader;

		AutoFillPipelineDesc(
			_ds
		);

		if (!CreatePipeline(_ds, "standard_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create default standard pipeline");
			return false;
		}
		
		standard_pipeline = { GetPipeline("standard_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this)};
		RenderFunc::DestroyShader(&VertShader);
		RenderFunc::DestroyShader(&FragShader);

		vert_src.clear();
		frag_src.clear();

		vert_src = ShaderParser::load_shader_file("../assets/shaders/cubemap.vert.glsl");
		frag_src = ShaderParser::load_shader_file("../assets/shaders/cubemap.frag.glsl");

		std::cout << vert_src;
		std::cout << frag_src;

		RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
		RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

		RaterizerState raterizer_state;
		raterizer_state.cull_mode = CullMode::FRONT;
		raterizer_state.fill_mode = FillMode::SOLID;


		ShaderResourceBinding cube_data;
		cube_data.shader_stage = ShaderStage::PIXEL_SHADER;
		cube_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		cube_data.resource_size = 0;
		cube_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		cube_data.resource_name = "CubeMap";
		cube_data.count = 1;
		cube_data.index = 0;
		cube_data.slot = 1;
		cube_data.resource_data_type = ShaderData::CUSTOM_DATA_TEXTURE;
			

		std::vector<ShaderResourceBinding> bindings = {
			projection,
			view,
			view_position,
			_test,
			cube_data
		};

		PipelineStateDesc _ds1 = {};
		_ds1.rateriser_state = raterizer_state;
		_ds1.resource_bindings_count = 5;
		_ds1.resource_bindings = bindings;
		_ds1.vertex_shader = &VertShader;
		_ds1.pixel_shader = &FragShader;

		AutoFillPipelineDesc(
			_ds1,
			true,
			false
		);
		

		
		if (!CreatePipeline(_ds1, "skybox_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create default skybox_pipeline");
			return false;
		}

		skybox_pipeline = { GetPipeline("skybox_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this)};
		RenderFunc::DestroyShader(&VertShader);
		RenderFunc::DestroyShader(&FragShader);
		return true;
	}
	PipelineManager* PipelineManager::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new PipelineManager();
		}

		return s_instance;
	}
	void PipelineManager::unloadDefault(GPipeline* pipeline)
	{
		if (pipeline->m_refCount > 0)
		{
			TRC_WARN("Can't unload a pipeline that is still in use");
			return;
		}

		pipeline->m_id = INVALID_ID;
		RenderFunc::DestroyPipeline(pipeline);
		pipeline->~GPipeline();
	}
}