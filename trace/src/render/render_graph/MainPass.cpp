#include "pch.h"

#include "MainPass.h"
#include "render/Graphics.h"
#include "render/Renderutils.h"
#include "render/RenderPassInfo.h"
#include "render/Renderer.h"
#include "render/GRenderPass.h"

namespace trace {



	void MainPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R8G8B8A8_SRBG;
			color_attach.initial_format = TextureFormat::UNKNOWN;
			color_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			color_attach.is_depth = false;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 1;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::UNKNOWN;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
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
			m_renderer->_avaliable_passes["MAIN_PASS"] = &m_renderPass;
		}
	}

	void MainPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
		auto pass = render_graph->AddPass("MAIN_PASS", GPU_QUEUE::GRAPHICS);
		color_output_index = pass_inputs.outputs[0];
		depth_index = pass_inputs.outputs[1];

		pass->CreateAttachmentOutput(
			render_graph->GetResource(color_output_index).resource_name,
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

				graph->ModifyTextureResource(graph->GetResource(color_output_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(depth_index).resource_name, desc);
			});
	}

	void MainPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}