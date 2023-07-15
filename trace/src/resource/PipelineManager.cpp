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

		if (auto_fill && desc.render_pass == nullptr)
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

		ShaderStruct::StructInfo proj = {};
		proj.resource_name = "projection";
		proj.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;
		proj.resource_size = sizeof(glm::mat4);

		ShaderStruct::StructInfo vw = {};
		vw.resource_name = "view";
		vw.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;
		vw.resource_size = sizeof(glm::mat4);

		ShaderStruct::StructInfo vw_pos = {};
		vw_pos.resource_name = "view_position";
		vw_pos.resource_data_type = ShaderData::CUSTOM_DATA_VEC3;
		vw_pos.resource_size = sizeof(glm::vec3);

		ShaderStruct::StructInfo tst = {};
		tst.resource_name = "light_data";
		tst.resource_data_type = ShaderData::CUSTOM_DATA_VEC4;
		tst.resource_size = sizeof(glm::ivec4);

		ShaderStruct scene_globals = {};
		scene_globals.count = 1;
		scene_globals.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		scene_globals.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		scene_globals.shader_stage = ShaderStage::VERTEX_SHADER;
		scene_globals.slot = 0;
		scene_globals.members = {
			proj,
			vw,
			vw_pos,
			tst
		};

		ShaderArray::ArrayInfo diff = {};
		diff.resource_name = "diffuse_map";
		diff.resource_data_type = ShaderData::MATERIAL_ALBEDO;
		diff.index = 0;

		ShaderArray::ArrayInfo spec = {};
		spec.resource_name = "specular_map";
		spec.resource_data_type = ShaderData::MATERIAL_SPECULAR;
		spec.index = 1;

		ShaderArray::ArrayInfo norm = {};
		norm.resource_name = "normal_map";
		norm.resource_data_type = ShaderData::MATERIAL_NORMAL;
		norm.index = 2;

		ShaderArray texs = {};
		texs.count = 3;
		texs.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		texs.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		texs.shader_stage = ShaderStage::PIXEL_SHADER;
		texs.slot = 1;
		texs.members = {
			diff,
			spec,
			norm
		};

		ShaderArray lgt = {};
		lgt.count = MAX_LIGHT_COUNT;
		lgt.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		lgt.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		lgt.shader_stage = ShaderStage::PIXEL_SHADER;
		lgt.slot = 3;
		lgt.name = "u_gLights";
		lgt.resource_size = sizeof(Light);


		ShaderStruct::StructInfo diff_col = {};
		diff_col.resource_name = "diffuse_color";
		diff_col.resource_data_type = ShaderData::MATERIAL_DIFFUSE_COLOR;
		diff_col.resource_size = sizeof(glm::vec4);

		ShaderStruct::StructInfo shine = {};
		shine.resource_name = "shininess";
		shine.resource_data_type = ShaderData::MATERIAL_SHININESS;
		shine.resource_size = sizeof(float);

		ShaderStruct inst_data = {};
		inst_data.count = 1;
		inst_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		inst_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		inst_data.shader_stage = ShaderStage::PIXEL_SHADER;
		inst_data.slot = 0;
		inst_data.members = {
			diff_col,
			shine
		};

		ShaderStruct::StructInfo mdl = {};
		mdl.resource_name = "model";
		mdl.resource_data_type = ShaderData::CUSTOM_DATA_MAT4;
		mdl.resource_size = sizeof(glm::mat4);

		ShaderStruct lcl = {};
		lcl.count = 1;
		lcl.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
		lcl.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		lcl.shader_stage = ShaderStage::VERTEX_SHADER;
		lcl.slot = 0;
		lcl.members = {
			mdl
		};

		ShaderStruct::StructInfo rst = {};
		rst.resource_name = "rest";
		rst.resource_data_type = ShaderData::CUSTOM_DATA_VEC4;
		rst.resource_size = sizeof(glm::ivec4);

		ShaderStruct rt = {};
		rt.count = 1;
		rt.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		rt.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		rt.shader_stage = ShaderStage::PIXEL_SHADER;
		rt.slot = 2;
		rt.members = {
			rst
		};

		ShaderResources scn_res = {};
		scn_res.resources.push_back({ scene_globals,{},{}, ShaderDataDef::STRUCTURE});
		scn_res.resources.push_back({ rt,{},{}, ShaderDataDef::STRUCTURE });
		scn_res.resources.push_back({ {},lgt,{}, ShaderDataDef::STRUCT_ARRAY });
		scn_res.resources.push_back({ {},texs,{}, ShaderDataDef::ARRAY });
		scn_res.resources.push_back({ inst_data,{},{}, ShaderDataDef::STRUCTURE });
		scn_res.resources.push_back({ lcl,{},{}, ShaderDataDef::STRUCTURE });

		PipelineStateDesc _ds;
		_ds.vertex_shader = &VertShader;
		_ds.pixel_shader = &FragShader;
		_ds.resources = scn_res;

		AutoFillPipelineDesc(
			_ds
		);
		_ds.render_pass = Renderer::get_instance()->GetRenderPass("MAIN_PASS");

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
			

		ShaderArray::ArrayInfo cb_t = {};
		cb_t.resource_name = "CubeMap";
		cb_t.resource_data_type = ShaderData::CUSTOM_DATA_TEXTURE;
		cb_t.index = 0;

		ShaderArray cb_d = {};
		cb_d.count = 1;
		cb_d.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		cb_d.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		cb_d.shader_stage = ShaderStage::PIXEL_SHADER;
		cb_d.slot = 1;
		cb_d.members = {
			cb_t
		};

		ShaderResources sky_res = {};
		sky_res.resources.push_back({ scene_globals,{},{}, ShaderDataDef::STRUCTURE });
		sky_res.resources.push_back({ {},cb_d,{}, ShaderDataDef::ARRAY });


		PipelineStateDesc _ds1 = {};
		_ds1.rateriser_state = raterizer_state;
		_ds1.vertex_shader = &VertShader;
		_ds1.pixel_shader = &FragShader;
		_ds1.resources = sky_res;

		AutoFillPipelineDesc(
			_ds1,
			true,
			false
		);
		_ds1.render_pass = Renderer::get_instance()->GetRenderPass("MAIN_PASS");
		

		
		if (!CreatePipeline(_ds1, "skybox_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create default skybox_pipeline");
			return false;
		}

		skybox_pipeline = { GetPipeline("skybox_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this)};
		RenderFunc::DestroyShader(&VertShader);
		RenderFunc::DestroyShader(&FragShader);


		vert_src.clear();
		frag_src.clear();

		vert_src = ShaderParser::load_shader_file("../assets/shaders/quad.vert.glsl");
		frag_src = ShaderParser::load_shader_file("../assets/shaders/custom.frag.glsl");

		std::cout << vert_src;
		std::cout << frag_src;

		RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
		RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

		ShaderArray::ArrayInfo _c = {};
		_c.index = 0;
		_c.resource_data_type = ShaderData::CUSTOM_DATA_TEXTURE;
		_c.resource_name = "color";

		ShaderArray col = {};
		col.count = 1;
		col.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		col.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		col.shader_stage = ShaderStage::PIXEL_SHADER;
		col.slot = 0;
		col.members = { _c };

		ShaderResources cus_res = {};
		cus_res.resources.push_back({ {}, col, {}, ShaderDataDef::ARRAY });

		PipelineStateDesc _ds2 = {};
		_ds2.vertex_shader = &VertShader;
		_ds2.pixel_shader = &FragShader;
		_ds2.resources = cus_res;
		_ds2.input_layout = Vertex2D::get_input_layout();
		
		
		AutoFillPipelineDesc(
			_ds2,
			false
		);
		_ds2.render_pass = Renderer::get_instance()->GetRenderPass("CUSTOM_PASS");
		_ds2.depth_sten_state = { false, false };


		if (!CreatePipeline(_ds2, "custom_pass_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create custom_pass_pipeline");
			return false;
		}

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