#include "pch.h"

#include "CustomPass.h"
#include "backends/Renderutils.h"
#include "render/Renderer.h"
#include "resource/ResourceSystem.h"
#include "render/GPipeline.h"
#include "RenderGraph.h"

namespace trace {



	void CustomPass::Init(Renderer* renderer)
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
			m_renderer->GetAvaliableRenderPasses()["CUSTOM_PASS"] = &m_renderPass;
		}

	}

	void CustomPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{

		color_input_index = pass_inputs.inputs[0];
		color_output_index = pass_inputs.outputs[0];

		RenderGraphPass* pass = render_graph->AddPass("CUSTOM_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentInput(color_input_index);

		pass->AddColorAttachmentOuput(color_output_index);



		pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);


				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"color",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(color_input_index)
				);
				RenderFunc::BindPipeline_(m_pipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());

				m_renderer->DrawQuad();
			});

		pass->SetResizeCB([&](RenderGraph* graph, RenderGraphPass* pass, uint32_t width, uint32_t height)
			{
				TextureDesc desc;
				desc.m_width = width;
				desc.m_height = height;

				graph->ModifyTextureResource(graph->GetResource(color_output_index).resource_name, desc);
			});


	}

	void CustomPass::ShutDown()
	{
		m_pipeline.release();
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}