#include "pch.h"

#include "BloomPass.h"
#include "render/Renderer.h"
#include "backends/Renderutils.h"
#include "render/ShaderParser.h"
#include "FrameData.h"

#include "resource/GenericAssetManager.h"
#include "render/GShader.h"
#include "RenderGraph.h"

namespace trace {

	extern UUID GetUUIDFromName(const std::string& name);

	void BloomPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			color_attach.initial_format = TextureFormat::UNKNOWN;
			color_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			color_attach.is_depth = false;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_DISCARD;
			color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo att_infos[] = {
				color_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = 1;
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;


			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);

			color_attach.initial_format = TextureFormat::UNKNOWN;
			RenderFunc::CreateRenderPass(&m_downSamplePass, pass_desc);
			color_attach.initial_format = TextureFormat::SHADER_READ;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			RenderFunc::CreateRenderPass(&m_upSamplePass, pass_desc);


			m_renderer->GetAvaliableRenderPasses()["BLOOM_PREFILTER_PASS"] = &m_renderPass;
			m_renderer->GetAvaliableRenderPasses()["BLOOM_DOWNSAMPLE_PASS"] = &m_downSamplePass;
			m_renderer->GetAvaliableRenderPasses()["BLOOM_UPSAMPLE_PASS"] = &m_upSamplePass;
		};

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		if (AppSettings::is_editor)
		{


			{

				Ref<GShader> VertShader;
				Ref<GShader> FragShader;

				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("bloom_prefilter.frag.glsl", "bloom_prefilter.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader;
				FragShader = frag_shader;

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader.get(), s_res);
				ShaderParser::generate_shader_resources(FragShader.get(), s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader.get();
				_ds2.pixel_shader = FragShader.get();
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("BLOOM_PREFILTER_PASS");
				_ds2.depth_sten_state = { false, false };
				_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


				m_prefilterPipeline = asset_manager->CreateAssetHandle<GPipeline>("bloom_prefilter_pass_pipeline", _ds2);
				if (!m_prefilterPipeline)
				{
					TRC_ERROR("Failed to initialize or create bloom_prefilter_pass_pipeline");
					return;
				}


			};

			{
				Ref<GShader> VertShader;
				Ref<GShader> FragShader;


				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("bloom_downsample.frag.glsl", "bloom_downsample.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader;
				FragShader = frag_shader;

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader.get(), s_res);
				ShaderParser::generate_shader_resources(FragShader.get(), s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader.get();
				_ds2.pixel_shader = FragShader.get();
				_ds2.resources = s_res;
				_ds2.input_layout = {};


				AutoFillPipelineDesc(
					_ds2,
					false
				);

				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("BLOOM_DOWNSAMPLE_PASS");
				_ds2.depth_sten_state = { false, false };
				_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


				m_downSamplePipeline = asset_manager->CreateAssetHandle<GPipeline>("bloom_downsample_pass_pipeline", _ds2);
				if (!m_downSamplePipeline)
				{
					TRC_ERROR("Failed to initialize or create bloom_downsample_pass_pipeline");
					return;
				}


			};

			{
				Ref<GShader> VertShader;
				Ref<GShader> FragShader;

				Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

				Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("bloom_upsample.frag.glsl", "bloom_upsample.frag.glsl", ShaderStage::PIXEL_SHADER);

				VertShader = vert_shader;
				FragShader = frag_shader;

				ShaderResources s_res = {};
				ShaderParser::generate_shader_resources(VertShader.get(), s_res);
				ShaderParser::generate_shader_resources(FragShader.get(), s_res);

				PipelineStateDesc _ds2 = {};
				_ds2.vertex_shader = VertShader.get();
				_ds2.pixel_shader = FragShader.get();
				_ds2.resources = s_res;
				_ds2.input_layout = {};

				AutoFillPipelineDesc(
					_ds2,
					false
				);
				_ds2.render_pass = Renderer::get_instance()->GetRenderPass("BLOOM_UPSAMPLE_PASS");
				_ds2.depth_sten_state = { false, false };
				_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };
				ColorBlendState clr_bld;
				clr_bld.alpha_op = BlendOp::BLEND_OP_ADD;
				clr_bld.alpha_to_blend_coverage = true;
				clr_bld.color_op = BlendOp::BLEND_OP_ADD;
				clr_bld.dst_alpha = BlendFactor::BLEND_ONE;
				clr_bld.src_alpha = BlendFactor::BLEND_ONE;
				clr_bld.dst_color = BlendFactor::BLEND_ONE;
				clr_bld.src_color = BlendFactor::BLEND_ONE;
				_ds2.blend_state = clr_bld;


				m_upSamplePipeline = asset_manager->CreateAssetHandle<GPipeline>("bloom_upsample_pass_pipeline", _ds2);
				if (!m_upSamplePipeline)
				{
					TRC_ERROR("Failed to initialize or create bloom_upsample_pass_pipeline");
					return;
				}

			};
		}
		else
		{
		UUID id = GetUUIDFromName("bloom_prefilter_pass_pipeline");
		m_prefilterPipeline = asset_manager->Load_Runtime<GPipeline>(id);
		id = GetUUIDFromName("bloom_downsample_pass_pipeline");
		m_downSamplePipeline = asset_manager->Load_Runtime<GPipeline>(id);
		id = GetUUIDFromName("bloom_upsample_pass_pipeline");
		m_upSamplePipeline = asset_manager->Load_Runtime<GPipeline>(id);

		}

	}
	void BloomPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}
	void BloomPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index)
	{
		prefilter(render_graph, black_board);
		downsample(render_graph, black_board);
		upsample(render_graph, black_board);
	}
	void BloomPass::ShutDown()
	{
		m_downSamplePipeline.release();
		m_prefilterPipeline.release();
		m_upSamplePipeline.release();
		RenderFunc::DestroyRenderPass(&m_upSamplePass);
		RenderFunc::DestroyRenderPass(&m_downSamplePass);
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
	void BloomPass::prefilter(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		FrameData& fd = black_board.get<FrameData>();
		BloomData& bd = black_board.add<BloomData>();

		bd.samples_count = 4;
		for (uint32_t i = 0; i < bd.samples_count; i++)
		{
			bd.bloom_samples[i] = INVALID_ID;
		}

		RenderGraphPass* pass = render_graph->AddPass("BLOOM_PREFILTER", GPU_QUEUE::GRAPHICS);

		uint32_t width = fd.frame_width / 2;
		uint32_t height = fd.frame_height / 2;

		TextureDesc color = {};
		color.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		color.m_attachmentType = AttachmentType::COLOR;
		color.m_flag = BindFlag::RENDER_TARGET_BIT;
		color.m_format = Format::R16G16B16A16_FLOAT;
		color.m_width = width;
		color.m_height = height;
		color.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
		color.m_mipLevels = color.m_numLayers = 1;
		color.m_usage = UsageFlag::DEFAULT;

		pass->AddColorAttachmentInput(fd.hdr_index);
		bd.bloom_samples[0] = pass->CreateAttachmentOutput("Bloom_sample_0", color);

		pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {

			

			float threshold = 1.01f;

			Viewport view_port = m_renderer->_viewPort;
			view_port.width = static_cast<float>(width);
			view_port.height = static_cast<float>(height);

			Rect2D rect = m_renderer->_rect;
			rect.right = width;
			rect.bottom = height;

			RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_prefilterPipeline.get());
			RenderFunc::BindRenderGraphTexture(
				render_graph,
				m_prefilterPipeline.get(),
				"u_srcTexture",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				render_graph->GetResource_ptr(fd.hdr_index)
			);
			RenderFunc::SetPipelineData(m_prefilterPipeline.get(), "threshold", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &threshold, sizeof(float));
			RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
			RenderFunc::BindRect(m_renderer->GetDevice(), rect);

			RenderFunc::BindPipeline_(m_prefilterPipeline.get());
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_prefilterPipeline.get());
			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
			RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_prefilterPipeline.get());

			});

	}
	void BloomPass::downsample(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		FrameData& fd = black_board.get<FrameData>();
		BloomData& bd = black_board.get<BloomData>();
		
		std::string pass_name("BLOOM_DOWNSAMPLE_0");
		size_t name_length = pass_name.length();
		std::string sample_name("Bloom_sample_0");
		size_t sample_name_length = sample_name.length();

		int width = fd.frame_width / 2;
		int height = fd.frame_height / 2;

		width /= 2;
		height /= 2;
		uint32_t sample_count = 1;

		TextureDesc color = {};
		color.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		color.m_attachmentType = AttachmentType::COLOR;
		color.m_flag = BindFlag::RENDER_TARGET_BIT;
		color.m_format = Format::R16G16B16A16_FLOAT;
		color.m_width = width;
		color.m_height = height;
		color.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
		color.m_mipLevels = color.m_numLayers = 1;
		color.m_usage = UsageFlag::DEFAULT;

		for (uint32_t i = 1; i < bd.samples_count; i++)
		{
			sample_count++;

			//TODO: Find A better way to generate name for each pass ----------------
			pass_name.replace(name_length - 1, 1, std::to_string(i));
			sample_name.replace(sample_name_length - 1, 1, std::to_string(i));
			RenderGraphPass* pass = render_graph->AddPass(pass_name, GPU_QUEUE::GRAPHICS);
			// -----------------------------------------------------------------------

			pass->AddColorAttachmentInput(bd.bloom_samples[i - 1]);
			bd.bloom_samples[i] = pass->CreateAttachmentOutput(sample_name, color);


			pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {


				Viewport view_port = m_renderer->_viewPort;
				view_port.width = static_cast<float>(width);
				view_port.height = static_cast<float>(height);

				Rect2D rect = m_renderer->_rect;
				rect.right = width;
				rect.bottom = height;

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_downSamplePipeline.get());
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_downSamplePipeline.get(),
					"down",
					ShaderResourceStage::RESOURCE_STAGE_INSTANCE,
					render_graph->GetResource_ptr(bd.bloom_samples[i - 1])
				);
				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);

				RenderFunc::BindPipeline_(m_downSamplePipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_downSamplePipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_downSamplePipeline.get());

				});

			if (width > 1) width /= 2;
			else break;
			if (height > 1) height /= 2;
			else break;

			color.m_width = width;
			color.m_height = height;
		}
		bd.samples_count = sample_count;

	}
	void BloomPass::upsample(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		FrameData& fd = black_board.get<FrameData>();
		BloomData& bd = black_board.get<BloomData>();

		std::string pass_name("BLOOM_UPSAMPLE_0");
		size_t name_length = pass_name.length();

		for (int i = bd.samples_count - 2; i >= 0; i--)
		{
			pass_name.replace(name_length - 1, 1, std::to_string(i));

			RenderGraphPass* pass = render_graph->AddPass(pass_name, GPU_QUEUE::GRAPHICS);
			pass->AddColorAttachmentInput(bd.bloom_samples[i + 1]);
			pass->AddColorAttachmentOuput(bd.bloom_samples[i]);

			

			pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {

				RenderGraphResource& res = render_graph->GetResource(bd.bloom_samples[i]);

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_upSamplePipeline.get());
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_upSamplePipeline.get(),
					"up",
					ShaderResourceStage::RESOURCE_STAGE_INSTANCE,
					render_graph->GetResource_ptr(bd.bloom_samples[i + 1])
				);
				float filter_radius = 1.0f;
				RenderFunc::SetPipelineData(
					m_upSamplePipeline.get(),
					"filterRadius",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&filter_radius,
					sizeof(float)
				);

				Viewport view_port = m_renderer->_viewPort;
				view_port.width = static_cast<float>(res.resource_data.texture.width);
				view_port.height = static_cast<float>(res.resource_data.texture.height);

				Rect2D rect = m_renderer->_rect;
				rect.right = res.resource_data.texture.width;
				rect.bottom = res.resource_data.texture.height;

				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);

				RenderFunc::BindPipeline_(m_upSamplePipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_upSamplePipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_upSamplePipeline.get());

				});

		}

		RenderGraphPass* pass = render_graph->AddPass("BLOOM_FINAL_UPSAMPLE", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentInput(bd.bloom_samples[0]);
		pass->AddColorAttachmentOuput(fd.hdr_index);



		pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {

			RenderGraphResource& res = render_graph->GetResource(fd.hdr_index);

			RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_upSamplePipeline.get());
			RenderFunc::BindRenderGraphTexture(
				render_graph,
				m_upSamplePipeline.get(),
				"up",
				ShaderResourceStage::RESOURCE_STAGE_INSTANCE,
				render_graph->GetResource_ptr(bd.bloom_samples[0])
			);
			float filter_radius = 1.0f;
			RenderFunc::SetPipelineData(
				m_upSamplePipeline.get(),
				"filterRadius",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&filter_radius,
				sizeof(float)
			);

			Viewport view_port = m_renderer->_viewPort;
			view_port.width = static_cast<float>(res.resource_data.texture.width);
			view_port.height = static_cast<float>(res.resource_data.texture.height);

			Rect2D rect = m_renderer->_rect;
			rect.right = res.resource_data.texture.width;
			rect.bottom = res.resource_data.texture.height;

			RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
			RenderFunc::BindRect(m_renderer->GetDevice(), rect);

			RenderFunc::BindPipeline_(m_upSamplePipeline.get());
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_upSamplePipeline.get());
			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
			RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_upSamplePipeline.get());

			});


	}
}