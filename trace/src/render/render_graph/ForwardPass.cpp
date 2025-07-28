#include "pch.h"

#include "ForwardPass.h"
#include "backends/Renderutils.h"
#include "render/ShaderParser.h"
#include "render/Renderer.h"
#include "FrameData.h"

// Temp =================
#include "backends/UIutils.h"
// ======================

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
			m_renderer->GetAvaliableRenderPasses()["FORWARD_PASS"] = &m_renderPass;
		};

	}
	void ForwardPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}
	void ForwardPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index)
	{
		FrameData& fd = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();

		RenderGraphPass* pass = render_graph->AddPass("FORWARD_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentOuput(fd.hdr_index);
		pass->SetDepthStencilInput(gbuffer_data.depth_index);
		uint32_t width = render_graph->GetResource(fd.hdr_index).resource_data.texture.width;
		uint32_t height = render_graph->GetResource(fd.hdr_index).resource_data.texture.height;

		pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {

			Viewport view_port = m_renderer->_viewPort;
			Rect2D rect = m_renderer->_rect;
			view_port.width = static_cast<float>(width);
			view_port.height = static_cast<float>(height);

			rect.right = width;
			rect.bottom = height;
			RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
			RenderFunc::BindRect(m_renderer->GetDevice(), rect);

			//m_renderer->RenderQuads(render_graph_index);
			m_renderer->RenderDebugData(render_graph_index); // Temp
			//m_renderer->RenderTextVerts(render_graph_index);
			


			});

	}
	void ForwardPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}