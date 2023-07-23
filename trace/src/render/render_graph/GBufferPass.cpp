#include "pch.h"

#include "GBufferPass.h"
#include "render/Renderutils.h"
#include "render/Renderer.h"

namespace trace {
	void GBufferPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo position_attach;
			position_attach.attachmant_index = 0;
			position_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			position_attach.initial_format = TextureFormat::UNKNOWN;
			position_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			position_attach.is_depth = false;
			position_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			position_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo normal_attach;
			normal_attach.attachmant_index = 1;
			normal_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			normal_attach.initial_format = TextureFormat::UNKNOWN;
			normal_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			normal_attach.is_depth = false;
			normal_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			normal_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo color_attach;
			color_attach.attachmant_index = 2;
			color_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			color_attach.initial_format = TextureFormat::UNKNOWN;
			color_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			color_attach.is_depth = false;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 3;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::UNKNOWN;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo att_infos[] = {
				position_attach,
				normal_attach,
				color_attach,
				depth_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = 4;
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;


			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);
			m_renderer->_avaliable_passes["GBUFFER_PASS"] = &m_renderPass;
		}
	}
	void GBufferPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
		auto pass = render_graph->AddPass("GBUFFER_PASS", GPU_QUEUE::GRAPHICS);
		position_index = pass_inputs.outputs[0];
		normal_index = pass_inputs.outputs[1];
		color_index = pass_inputs.outputs[2];
		depth_index = pass_inputs.outputs[3];

		pass->CreateAttachmentOutput(
			render_graph->GetResource(position_index).resource_name,
			{}
		);
		pass->CreateAttachmentOutput(
			render_graph->GetResource(normal_index).resource_name,
			{}
		);
		pass->CreateAttachmentOutput(
			render_graph->GetResource(color_index).resource_name,
			{}
		);

		pass->CreateDepthAttachmentOutput(
			render_graph->GetResource(depth_index).resource_name,
			{}
		);

		pass->SetRunCB([&](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);


				for (uint32_t i = 0; i < m_renderer->m_listCount; i++)
				{
					for (Command& cmd : m_renderer->m_cmdList[i]._commands)
					{
						cmd.func(cmd.params);
					}
				}
			});

		pass->SetResizeCB([&](RenderGraph* graph, RenderGraphPass* pass, uint32_t width, uint32_t height)
			{
				TextureDesc desc;
				desc.m_width = width;
				desc.m_height = height;

				graph->ModifyTextureResource(graph->GetResource(position_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(normal_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(color_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(depth_index).resource_name, desc);
			});
	}
	void GBufferPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}