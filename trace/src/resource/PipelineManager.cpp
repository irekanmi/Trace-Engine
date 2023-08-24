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


		std::string vert_src;
		std::string frag_src;
		GShader VertShader;
		GShader FragShader;

		{
				vert_src = ShaderParser::load_shader_file("../assets/shaders/cubemap.vert.glsl");
				frag_src = ShaderParser::load_shader_file("../assets/shaders/cubemap.frag.glsl");

				std::cout << vert_src;
				std::cout << frag_src;

				RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
				RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

				RaterizerState raterizer_state;
				raterizer_state.cull_mode = CullMode::FRONT;
				raterizer_state.fill_mode = FillMode::SOLID;

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(&VertShader, s_res);
				ShaderParser::generate_shader_resources(&FragShader, s_res);

				PipelineStateDesc _ds1 = {};
				_ds1.rateriser_state = raterizer_state;
				_ds1.vertex_shader = &VertShader;
				_ds1.pixel_shader = &FragShader;
				_ds1.resources = s_res;

				AutoFillPipelineDesc(
					_ds1,
					true,
					false
				);
				_ds1.render_pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");



				if (!CreatePipeline(_ds1, "skybox_pipeline"))
				{
					TRC_ERROR("Failed to initialize or create default skybox_pipeline");
					return false;
				}

				skybox_pipeline = { GetPipeline("skybox_pipeline") , BIND_RESOURCE_UNLOAD_FN(PipelineManager::unloadDefault, this) };
				RenderFunc::DestroyShader(&VertShader);
				RenderFunc::DestroyShader(&FragShader);


				vert_src.clear();
				frag_src.clear();
			}


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