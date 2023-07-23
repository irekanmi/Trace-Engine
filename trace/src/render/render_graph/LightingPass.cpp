#include "pch.h"

#include "LightingPass.h"
#include "render/Renderer.h"
#include "RenderGraph.h"
#include "render/Renderutils.h"
#include "render/GPipeline.h"
#include "resource/ResourceSystem.h"


namespace trace {



	void LightingPass::Init(Renderer* renderer)
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
			m_renderer->_avaliable_passes["LIGHTING_PASS"] = &m_renderPass;
		}

	}

	void LightingPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{

		auto pass = render_graph->AddPass("LIGHTING_PASS", GPU_QUEUE::GRAPHICS);
		color_output_index = pass_inputs.outputs[0];
		gPosition_index = pass_inputs.inputs[0];
		gNormal_index = pass_inputs.inputs[1];
		gColor_index = pass_inputs.inputs[2];

		pass->CreateAttachmentOutput(
			render_graph->GetResource(color_output_index).resource_name,
			{}
		);

		pass->AddColorAttachmentInput(render_graph->GetResource(gPosition_index).resource_name);
		pass->AddColorAttachmentInput(render_graph->GetResource(gNormal_index).resource_name);
		pass->AddColorAttachmentInput(render_graph->GetResource(gColor_index).resource_name);

		m_pipeline = ResourceSystem::get_instance()->GetPipeline("lighting_pass_pipeline");

		pass->SetRunCB([=](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);


				RenderFunc::BindRenderGraphResource(
					render_graph,
					m_pipeline.get(),
					"g_bufferData0",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&render_graph->GetResource(gPosition_index),
					0
				);

				RenderFunc::BindRenderGraphResource(
					render_graph,
					m_pipeline.get(),
					"g_bufferData1",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&render_graph->GetResource(gNormal_index),
					1
				);

				RenderFunc::BindRenderGraphResource(
					render_graph,
					m_pipeline.get(),
					"g_bufferData2",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&render_graph->GetResource(gColor_index),
					2
				);

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"rest",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&m_renderer->render_mode,
					sizeof(glm::ivec4)
				);

				RenderFunc::SetPipelineData(
					m_pipeline.get(), 
					"u_gLights", 
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL, 
					m_renderer->lights, 
					sizeof(Light) * MAX_LIGHT_COUNT
				);

				RenderFunc::SetPipelineData(m_pipeline.get(), "view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &m_renderer->_camera->GetPosition(), sizeof(glm::vec3));
				RenderFunc::SetPipelineData(m_pipeline.get(), "light_data", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &m_renderer->light_data, sizeof(glm::ivec4));

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

	void LightingPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}