#include "pch.h"

#include "render/GTexture.h"
#include "SSAO.h"
#include "render/Renderutils.h"
#include "render/Renderer.h"


namespace trace {
	void SSAO::Init(Renderer* renderer)
	{

		m_renderer = renderer;

		AttachmentInfo ssao_attach;
		ssao_attach.attachmant_index = 0;
		ssao_attach.attachment_format = Format::R16_FLOAT;
		ssao_attach.initial_format = TextureFormat::UNKNOWN;
		ssao_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
		ssao_attach.is_depth = false;
		ssao_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
		ssao_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

		AttachmentInfo att_infos[] = {
			ssao_attach
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
		m_renderer->_avaliable_passes["SSAO_MAIN_PASS"] = &m_renderPass;


		RenderFunc::CreateRenderPass(&ssao_blur, pass_desc);
		m_renderer->_avaliable_passes["SSAO_BLUR_PASS"] = &ssao_blur;

	}
	void SSAO::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{

		RenderGraphPass* main_pass = render_graph->AddPass("SSAO_MAIN_PASS", GPU_QUEUE::GRAPHICS);
		RenderGraphPass* blur_pass = render_graph->AddPass("SSAO_BLUR_PASS", GPU_QUEUE::GRAPHICS);


	}
	void SSAO::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&ssao_blur);
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}
