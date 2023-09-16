#include "pch.h"

#include "PipelineManager.h"
#include "core/Coretypes.h"
#include "render/Renderer.h"
#include "core/FileSystem.h"
#include "render/GShader.h"
#include "render/ShaderParser.h"
#include "render/Renderutils.h"
#include "ShaderManager.h"

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
		skybox_pipeline.~Ref();
		if (!m_pipelines.empty())
		{
			
			for (GPipeline& pipeline : m_pipelines)
			{
				if (pipeline.m_id == INVALID_ID)
					continue;
				RenderFunc::DestroyPipeline(&pipeline);
				TRC_TRACE("Pipeline not destroyed id: {}", pipeline.m_id);
			}
			m_pipelines.clear();
		}

	}
	bool PipelineManager::CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill)
	{



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

		GShader* VertShader;
		GShader* FragShader;

		{
				RasterizerState raterizer_state;
				raterizer_state.cull_mode = CullMode::FRONT;
				raterizer_state.fill_mode = FillMode::SOLID;


				ShaderManager::get_instance()->CreateShader("cubemap.vert.glsl", ShaderStage::VERTEX_SHADER);
				ShaderManager::get_instance()->CreateShader("cubemap.frag.glsl", ShaderStage::PIXEL_SHADER);
				VertShader = ShaderManager::get_instance()->GetShader("cubemap.vert.glsl");
				FragShader = ShaderManager::get_instance()->GetShader("cubemap.frag.glsl");

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader, s_res);
				ShaderParser::generate_shader_resources(FragShader, s_res);

				PipelineStateDesc _ds1 = {};
				_ds1.rasteriser_state = raterizer_state;
				_ds1.vertex_shader = VertShader;
				_ds1.pixel_shader = FragShader;
				_ds1.resources = s_res;

				AutoFillPipelineDesc(
					_ds1,
					true,
					false
				);
				_ds1.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");



				if (!CreatePipeline(_ds1, "skybox_pipeline"))
				{
					TRC_ERROR("Failed to initialize or create default skybox_pipeline");
					return false;
				}

				skybox_pipeline = { GetPipeline("skybox_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this) };
			}

		{

			ShaderManager::get_instance()->CreateShader("lights.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("lights.frag.glsl", ShaderStage::PIXEL_SHADER);
			VertShader = ShaderManager::get_instance()->GetShader("lights.vert.glsl");
			FragShader = ShaderManager::get_instance()->GetShader("lights.frag.glsl");

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(VertShader, s_res);
			ShaderParser::generate_shader_resources(FragShader, s_res);


			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = VertShader;
			_ds2.pixel_shader = FragShader;
			_ds2.resources = s_res;
			_ds2.input_layout = Vertex::get_input_layout();


			AutoFillPipelineDesc(
				_ds2,
				false
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");


			if (!CreatePipeline(_ds2, "light_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create light_pipeline");
				return false;
			}
			light_pipeline = { GetPipeline("light_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this) };;

		};
		{
			ShaderManager::get_instance()->CreateShader("quad.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("quad.frag.glsl", ShaderStage::PIXEL_SHADER);
			GShader* VertShader = ShaderManager::get_instance()->GetShader("quad.vert.glsl");
			GShader* FragShader = ShaderManager::get_instance()->GetShader("quad.frag.glsl");

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(VertShader, s_res);
			ShaderParser::generate_shader_resources(FragShader, s_res);

			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = VertShader;
			_ds2.pixel_shader = FragShader;
			_ds2.resources = s_res;
			_ds2.input_layout = {};


			AutoFillPipelineDesc(
				_ds2,
				false
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");
			_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };


			if (!CreatePipeline(_ds2, "quad_batch_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
				return false;
			}
		};

		{
			ShaderManager::get_instance()->CreateShader("text.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("text_MSDF.frag.glsl", ShaderStage::PIXEL_SHADER);
			GShader* VertShader = ShaderManager::get_instance()->GetShader("text.vert.glsl");
			GShader* FragShader = ShaderManager::get_instance()->GetShader("text_MSDF.frag.glsl");

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(VertShader, s_res);
			ShaderParser::generate_shader_resources(FragShader, s_res);

			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = VertShader;
			_ds2.pixel_shader = FragShader;
			_ds2.resources = s_res;
			_ds2.input_layout = {};


			AutoFillPipelineDesc(
				_ds2,
				false
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");
			_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };
			ColorBlendState clr_bld;
			clr_bld.alpha_op = BlendOp::BLEND_OP_ADD;
			clr_bld.alpha_to_blend_coverage = true;
			clr_bld.color_op = BlendOp::BLEND_OP_ADD;
			clr_bld.dst_alpha = BlendFactor::BLEND_ONE;
			clr_bld.src_alpha = BlendFactor::BLEND_ONE;
			clr_bld.dst_color = BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
			clr_bld.src_color = BlendFactor::BLEND_ONE;
			_ds2.blend_state = clr_bld;
			_ds2.depth_sten_state = { false, false, 0.0f, 1.0f };


			if (!CreatePipeline(_ds2, "text_batch_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
				return false;
			}
		};

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