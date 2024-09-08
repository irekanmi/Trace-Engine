#include "pch.h"

#include "PipelineManager.h"
#include "core/Coretypes.h"
#include "render/Renderer.h"
#include "core/FileSystem.h"
#include "render/GShader.h"
#include "render/ShaderParser.h"
#include "backends/Renderutils.h"
#include "ShaderManager.h"
#include "serialize/PipelineSerializer.h"
#include "core/Utils.h"
#include "core/Coretypes.h"

namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);
	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);


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
				TRC_TRACE("Pipeline not destroyed, name : {}, RefCount : {}", pipeline.GetName(), pipeline.m_refCount);
			}
			m_pipelines.clear();
		}

	}
	Ref<GPipeline> PipelineManager::CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill)
	{
		Ref<GPipeline> result;
		GPipeline* _pipe = nullptr;
		result = GetPipeline(name);
		if (result)
		{
			TRC_WARN("{} name has already been created", name);
			return result;
		}
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
		uint32_t& hash = m_hashtable.Get_Ref(name);
		if (hash == INVALID_ID)
		{
			return result;
		}

		_pipe = &m_pipelines[hash];
		if (_pipe->m_id == INVALID_ID)
		{
			TRC_WARN("{} has already been destroyed", name);
			hash = INVALID_ID;
			return result;
		}
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
	bool PipelineManager::RecreatePipeline(Ref<GPipeline> pipeline, PipelineStateDesc desc)
	{
		RenderFunc::DestroyPipeline(pipeline.get());

		if (!RenderFunc::CreatePipeline(pipeline.get(), desc))
		{
			TRC_ERROR("Failed to create pipeline {}", pipeline->GetName());
			pipeline->m_id = INVALID_ID;
			pipeline->GetRenderHandle()->m_internalData = nullptr;
			return false;
		}
		if (!RenderFunc::InitializePipeline(pipeline.get()))
		{
			TRC_ERROR("Failed to initialize pipeline {}", pipeline->GetName());
			RenderFunc::DestroyPipeline(pipeline.get());
			pipeline->m_id = INVALID_ID;
			pipeline->GetRenderHandle()->m_internalData = nullptr;
			return false;
		}

		return true;
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
	Ref<GPipeline> PipelineManager::LoadPipeline_Runtime(UUID id)
	{
		Ref<GPipeline> result;
		GPipeline* _pipe = nullptr;

		auto it = m_assetMap.find(id);
		if (it == m_assetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		std::string name = GetNameFromUUID(id);
		result = GetPipeline(name);
		if (result)
		{
			TRC_WARN("{} name has already been created", name);
			return result;
		}
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			if (m_pipelines[i].m_id == INVALID_ID)
			{
				std::string bin_dir;
				FindDirectory(AppSettings::exe_path, "Data/trpip.trbin", bin_dir);
				FileStream stream(bin_dir, FileMode::READ);

				stream.SetPosition(it->second.offset);
				UUID vert_id = 0;
				stream.Read<UUID>(vert_id);
				UUID frag_id = 0;
				stream.Read<UUID>(frag_id);
				PipelineStateDesc desc;
				desc.vertex_shader = ShaderManager::get_instance()->LoadShader_Runtime(vert_id).get();
				desc.pixel_shader = ShaderManager::get_instance()->LoadShader_Runtime(frag_id).get();

				stream.Read<uint32_t>(desc.input_layout.stride);
				stream.Read<InputClassification>(desc.input_layout.input_class);
				int input_layout_element_count = 0;
				stream.Read<int>(input_layout_element_count);
				desc.input_layout.elements.resize(input_layout_element_count);
				stream.Read(desc.input_layout.elements.data(), input_layout_element_count * sizeof(InputLayout::Element));

				stream.Read<RasterizerState>(desc.rasteriser_state);

				stream.Read<DepthStencilState>(desc.depth_sten_state);

				stream.Read<ColorBlendState>(desc.blend_state);

				stream.Read<PRIMITIVETOPOLOGY>(desc.topology);

				stream.Read<Viewport>(desc.view_port);

				stream.Read<uint32_t>(desc.subpass_index);


				char buf[128] = { 0 };
				int pass_name_lenght = 0;
				stream.Read<int>(pass_name_lenght);
				stream.Read(buf, pass_name_lenght);
				desc.render_pass = Renderer::get_instance()->GetRenderPass(buf);

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(desc.vertex_shader, s_res);
				ShaderParser::generate_shader_resources(desc.pixel_shader, s_res);
				desc.resources = s_res;

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
				break;
			}
		}

		result = { _pipe, BIND_RENDER_COMMAND_FN(PipelineManager::Unload) };
		return result;
	}
	bool PipelineManager::BuildDefaultPipelines(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{


		for (GPipeline& _p : m_pipelines)
		{
			if (_p.m_id != INVALID_ID)
			{
				Ref<GPipeline> pipeline = { &_p, BIND_RENDER_COMMAND_FN(PipelineManager::Unload) };
				PipelineSerializer::Serialize(pipeline, stream, map);

			}
			
		}

		return true;
	}
	bool PipelineManager::BuildDefaultPipelineShaders(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map)
	{


		for (GPipeline& _p : m_pipelines)
		{
			if (_p.m_id != INVALID_ID)
			{
				Ref<GPipeline> pipeline = { &_p, BIND_RENDER_COMMAND_FN(PipelineManager::Unload) };
				PipelineStateDesc ds = pipeline->GetDesc();
				Ref<GShader> vert = ShaderManager::get_instance()->GetShader(ds.vertex_shader->GetName());
				Ref<GShader> frag = ShaderManager::get_instance()->GetShader(ds.pixel_shader->GetName());
				PipelineSerializer::SerializeShader(vert, stream, map);
				PipelineSerializer::SerializeShader(frag, stream, map);
			}

		}

		return false;
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
			ColorBlendState clr_bld;
			clr_bld.alpha_op = BlendOp::BLEND_OP_ADD;
			clr_bld.alpha_to_blend_coverage = true;
			clr_bld.color_op = BlendOp::BLEND_OP_ADD;
			clr_bld.dst_alpha = BlendFactor::BLEND_ONE;
			clr_bld.src_alpha = BlendFactor::BLEND_ONE;
			clr_bld.dst_color = BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
			clr_bld.src_color = BlendFactor::BLEND_SRC_ALPHA;
			_ds2.blend_state = clr_bld;
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
			clr_bld.src_color = BlendFactor::BLEND_SRC_ALPHA;
			_ds2.blend_state = clr_bld;
			_ds2.depth_sten_state = { true, false, 0.0f, 1.0f };

			text_batch_pipeline = CreatePipeline(_ds2, "text_batch_pipeline");
			if (!text_batch_pipeline)
			{
				TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
				return false;
			}
		};

		{
			GShader* VertShader = ShaderManager::get_instance()->CreateShader("text_v.vert.glsl", ShaderStage::VERTEX_SHADER).get();
			GShader* FragShader = ShaderManager::get_instance()->CreateShader("text_f.frag.glsl", ShaderStage::PIXEL_SHADER).get();

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
			_ds2.input_layout = TextVertex::get_input_layout();
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
			_ds2.depth_sten_state = { true, false, 0.0f, 1.0f };

			text_pipeline = CreatePipeline(_ds2, "text_pipeline");
			if (!text_pipeline)
			{
				TRC_ERROR("Failed to initialize or create text_pipeline");
				return false;
			}
		};

		{
			ShaderManager::get_instance()->CreateShader("debug_line.vert.glsl", ShaderStage::VERTEX_SHADER);
			ShaderManager::get_instance()->CreateShader("debug_line.frag.glsl", ShaderStage::PIXEL_SHADER);
			GShader* VertShader = ShaderManager::get_instance()->GetShader("debug_line.vert.glsl").get();
			GShader* FragShader = ShaderManager::get_instance()->GetShader("debug_line.frag.glsl").get();

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS"); // TODO: Create custom debug pass
			_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };
			_ds2.topology = PRIMITIVETOPOLOGY::LINE_LIST;
			ColorBlendState clr_bld;
			clr_bld.alpha_op = BlendOp::BLEND_OP_ADD;
			clr_bld.alpha_to_blend_coverage = true;
			clr_bld.color_op = BlendOp::BLEND_OP_ADD;
			clr_bld.dst_alpha = BlendFactor::BLEND_ONE;
			clr_bld.src_alpha = BlendFactor::BLEND_ONE;
			clr_bld.dst_color = BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA;
			clr_bld.src_color = BlendFactor::BLEND_SRC_ALPHA;
			_ds2.blend_state = clr_bld;
			_ds2.depth_sten_state = { true, false, 0.0f, 1.0f };

			debug_line_pipeline = CreatePipeline(_ds2, "debug_line_pipeline");
			if (!debug_line_pipeline)
			{
				TRC_ERROR("Failed to initialize or create debug_line_pipeline");
				return false;
			}
		};

		{

			Ref<GShader> VertShader = ShaderManager::get_instance()->CreateShader("trace_core.shader.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = ShaderManager::get_instance()->CreateShader("parallelex_brdf.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			gbuffer_pipeline->SetPipelineType(PipelineType::Surface_Material);


		};

		return true;
	}
	bool PipelineManager::LoadDefaults_Runtime()
	{
		UUID id = GetUUIDFromName("skybox_pipeline");
		skybox_pipeline = LoadPipeline_Runtime(id);

		id = GetUUIDFromName("light_pipeline");
		light_pipeline = LoadPipeline_Runtime(id);

		id = GetUUIDFromName("quad_batch_pipeline");
		quad_pipeline = LoadPipeline_Runtime(id);

		id = GetUUIDFromName("text_batch_pipeline");
		text_batch_pipeline = LoadPipeline_Runtime(id);

		id = GetUUIDFromName("text_pipeline");
		text_pipeline = LoadPipeline_Runtime(id);

		id = GetUUIDFromName("debug_line_pipeline");
		debug_line_pipeline = LoadPipeline_Runtime(id);

		id = GetUUIDFromName("gbuffer_pipeline");
		gbuffer_pipeline = LoadPipeline_Runtime(id);

		

		return true;
	}
	PipelineManager* PipelineManager::get_instance()
	{
		static PipelineManager* s_instance = new PipelineManager;

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