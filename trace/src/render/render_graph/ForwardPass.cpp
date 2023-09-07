#include "pch.h"

#include "ForwardPass.h"
#include "render/Renderutils.h"
#include "render/ShaderParser.h"
#include "render/Renderer.h"
#include "FrameData.h"

namespace trace {
	void ForwardPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;
		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			color_attach.initial_format = TextureFormat::SHADER_READ;
			color_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			color_attach.is_depth = false;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 1;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo att_infos[] = {
				color_attach,
				depth_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = 2;
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;


			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);
			m_renderer->_avaliable_passes["FORWARD_PASS"] = &m_renderPass;
		};

	}
	void ForwardPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}
	void ForwardPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		FrameData& fd = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();

		RenderGraphPass* pass = render_graph->AddPass("FORWARD_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentOuput(fd.hdr_index);
		pass->SetDepthStencilInput(gbuffer_data.depth_index);

		pass->SetRunCB([=](std::vector<uint32_t>& inputs) {

			RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
			RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);

			m_renderer->RenderLights();


			});

	}
	void ForwardPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}