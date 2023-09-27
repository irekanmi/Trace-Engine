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
	Ref<GPipeline> PipelineManager::CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill)
	{
		Ref<GPipeline> result;
		GPipeline* _pipe = nullptr;
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			if (m_pipelines[i].m_id == INVALID_ID)
			{
				if (!RenderFunc::CreatePipeline(&m_pipelines[i], desc))
				{
					TRC_ERROR("Failed to create pipeline {}", name);
					return result;
				}
				if (!RenderFunc::InitializePipeline(&m_pipelines[i]))
				{
					TRC_ERROR("Failed to initialize pipeline {}", name);
					RenderFunc::DestroyPipeline(&m_pipelines[i]);
					return result;
				}
				m_pipelines[i].m_id = i;
				m_hashtable.Set(name, i);
				_pipe = &m_pipelines[i];
				_pipe->m_path = name;
				break;
			}
		}

		result = { _pipe, BIND_RENDER_COMMAND_FN(PipelineManager::Unload) };
		return result;
	}
	Ref<GPipeline> PipelineManager::GetPipeline(const std::string& name)
	{
		Ref<GPipeline> result;
		GPipeline* _pipe = nullptr;
		uint32_t hash = m_hashtable.Get(name);
		if (hash == INVALID_ID)
		{
			return result;
		}

		_pipe = &m_pipelines[hash];
		result = { _pipe, BIND_RENDER_COMMAND_FN(PipelineManager::Unload) };
		return result;
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
				VertShader = ShaderManager::get_instance()->GetShader("cubemap.vert.glsl").get();
				FragShader = ShaderManager::get_instance()->GetShader("cubemap.frag.glsl").get();

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
				skybox_pipeline = CreatePipeline(_ds1, "skybox_pipeline");
				if (!skybox_pipeline)
				{
					TRC_ERROR("Failed to initialize or create default skybox_pipeline");
					return false;
				}
			}

		{

			ShaderManager::get_instance()->CreateShader("lights.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("lights.frag.glsl", ShaderStage::PIXEL_SHADER);
			VertShader = ShaderManager::get_instance()->GetShader("lights.vert.glsl").get();
			FragShader = ShaderManager::get_instance()->GetShader("lights.frag.glsl").get();

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

			light_pipeline = CreatePipeline(_ds2, "light_pipeline");
			if (!light_pipeline)
			{
				TRC_ERROR("Failed to initialize or create light_pipeline");
				return false;
			}

		};
		{
			ShaderManager::get_instance()->CreateShader("quad.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("quad.frag.glsl", ShaderStage::PIXEL_SHADER);
			GShader* VertShader = ShaderManager::get_instance()->GetShader("quad.vert.glsl").get();
			GShader* FragShader = ShaderManager::get_instance()->GetShader("quad.frag.glsl").get();

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

			quad_pipeline = CreatePipeline(_ds2, "quad_batch_pipeline");
			if (!quad_pipeline)
			{
				TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
				return false;
			}
		};

		{
			ShaderManager::get_instance()->CreateShader("text.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("text_MSDF.frag.glsl", ShaderStage::PIXEL_SHADER);
			GShader* VertShader = ShaderManager::get_instance()->GetShader("text.vert.glsl").get();
			GShader* FragShader = ShaderManager::get_instance()->GetShader("text_MSDF.frag.glsl").get();

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

			text_pipeline = CreatePipeline(_ds2, "text_batch_pipeline");
			if (!text_pipeline)
			{
				TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
				return false;
			}
		};

		{

			Ref<GShader> VertShader = ShaderManager::get_instance()->CreateShader("trace_core.shader.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = ShaderManager::get_instance()->CreateShader("g_buffer.frag.glsl", ShaderStage::PIXEL_SHADER);

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(VertShader.get(), s_res);
			ShaderParser::generate_shader_resources(FragShader.get(), s_res);

			PipelineStateDesc _ds;
			_ds.vertex_shader = VertShader.get();
			_ds.pixel_shader = FragShader.get();
			_ds.resources = s_res;

			AutoFillPipelineDesc(
				_ds
			);
			_ds.render_pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");
			_ds.blend_state.alpha_to_blend_coverage = false;

			gbuffer_pipeline = CreatePipeline(_ds, "gbuffer_pipeline");
			if (!gbuffer_pipeline)
			{
				TRC_ERROR("Failed to initialize or create default gbuffer pipeline");
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