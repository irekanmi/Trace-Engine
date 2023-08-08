#include "pch.h"

#include "render/GTexture.h"
#include "SSAO.h"
#include "render/Renderutils.h"
#include "render/Renderer.h"
#include "FrameData.h"




namespace trace {
	void SSAO::Init(Renderer* renderer)
	{

		m_renderer = renderer;

		{
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
		};

		{
			TextureDesc gPos = {};
			gPos.m_addressModeU = gPos.m_addressModeV = gPos.m_addressModeW = AddressMode::REPEAT;
			gPos.m_attachmentType = AttachmentType::COLOR;
			gPos.m_flag = BindFlag::RENDER_TARGET_BIT;
			gPos.m_format = Format::R16_FLOAT;
			gPos.m_width = 800;
			gPos.m_height = 600;
			gPos.m_minFilterMode = gPos.m_magFilterMode = FilterMode::LINEAR;
			gPos.m_mipLevels = gPos.m_numLayers = 1;
			gPos.m_usage = UsageFlag::DEFAULT;

			ssao_main_desc = gPos;

		};

		{

		};

		{
			for (uint32_t i = 0; i < MAX_NUM_KERNEL; i++)
			{
				glm::vec3 rand_vec = glm::vec3(
					
				);
			}
		};

	}
	void SSAO::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{

		RenderGraphPass* main_pass = render_graph->AddPass("SSAO_MAIN_PASS", GPU_QUEUE::GRAPHICS);
		RenderGraphPass* blur_pass = render_graph->AddPass("SSAO_BLUR_PASS", GPU_QUEUE::GRAPHICS);

		FrameData& fd = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();
		SSAOData& ssao_data = black_board.add<SSAOData>();

		ssao_main_desc.m_width = fd.frame_width;
		ssao_main_desc.m_height = fd.frame_height;
		

		main_pass->AddColorAttachmentInput(
			render_graph->GetResource(gbuffer_data.position_index).resource_name
		);

		main_pass->AddColorAttachmentInput(
			render_graph->GetResource(gbuffer_data.normal_index).resource_name
		);

		ssao_data.ssao_main = main_pass->CreateAttachmentOutput(
			"ssao_main",
			ssao_main_desc
		);

		blur_pass->AddColorAttachmentInput(
			"ssao_main"
		);

		ssao_data.ssao_blur = blur_pass->CreateAttachmentOutput(
			"ssao_blur",
			ssao_main_desc
		);

		main_pass->SetRunCB([&](std::vector<uint32_t>& inputs) {
				
			RenderGraphResource& pos_res = render_graph->GetResource(gbuffer_data.position_index);
			RenderGraphResource& norm_res = render_graph->GetResource(gbuffer_data.normal_index);

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"u_kernel",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				m_kernel.data(),
				static_cast<uint32_t>(m_kernel.size() * sizeof(glm::vec3))
			);

			glm::vec2 frame_size(static_cast<float>(fd.frame_width), static_cast<float>(fd.frame_width));

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"frame_size",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&frame_size,
				sizeof(glm::vec2)
			);

			RenderFunc::SetPipelineTextureData(
				m_pipeline.get(),
				"u_noiseTexture",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&noise_tex
			);

			RenderFunc::BindRenderGraphResource(
				render_graph,
				m_pipeline.get(),
				"g_bufferData0",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&pos_res
			);

			RenderFunc::BindRenderGraphResource(
				render_graph,
				m_pipeline.get(),
				"g_bufferData1",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&norm_res
			);

			RenderFunc::BindPipeline_(m_pipeline.get());

			m_renderer->DrawQuad();


			});

		blur_pass->SetRunCB([&](std::vector<uint32_t>& inputs) {

			RenderGraphResource& res = render_graph->GetResource(ssao_data.ssao_main);

			RenderFunc::BindRenderGraphResource(
				render_graph,
				m_blurPipe.get(),
				res.resource_name,
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&res
			);

			RenderFunc::BindPipeline_(m_blurPipe.get());

			m_renderer->DrawQuad();

			});

	}
	void SSAO::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&ssao_blur);
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}
