#include "pch.h"

#include "BloomPass.h"
#include "render/Renderer.h"
#include "render/Renderutils.h"
#include "render/ShaderParser.h"
#include "FrameData.h"

namespace trace {
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
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
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
			RenderFunc::CreateRenderPass(&m_upSamplePass, pass_desc);


			m_renderer->_avaliable_passes["BLOOM_PREFILTER_PASS"] = &m_renderPass;
			m_renderer->_avaliable_passes["BLOOM_DOWNSAMPLE_PASS"] = &m_downSamplePass;
			m_renderer->_avaliable_passes["BLOOM_UPSAMPLE_PASS"] = &m_upSamplePass;
		};



	}
	void BloomPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}
	void BloomPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
	}
	void BloomPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_upSamplePass);
		RenderFunc::DestroyRenderPass(&m_downSamplePass);
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
	void BloomPass::prefilter(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		FrameData& fd = black_board.get<FrameData>();
		BloomData& bd = black_board.add<BloomData>();

		bd.samples_count = 5;
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

		pass->SetRunCB([=](std::vector<uint32_t>& inputs) {

			RenderFunc::BindRenderGraphTexture(
				render_graph,
				m_prefilterPipeline.get(),
				"prefilter_Texture",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				render_graph->GetResource_ptr(fd.hdr_index)
			);

			float threshold = 1.01f;
			RenderFunc::SetPipelineData(m_prefilterPipeline.get(), "threshold", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &threshold, sizeof(float));

			RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
			RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);

			RenderFunc::BindVertexBuffer(m_renderer->GetDevice(), &m_renderer->quadBuffer);
			RenderFunc::BindPipeline_(m_prefilterPipeline.get());
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_prefilterPipeline.get());
			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);

			});

	}
	void BloomPass::downsample(RenderGraph* render_graph, RGBlackBoard& black_board)
	{


	}
	void BloomPass::upsample(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
	}
}