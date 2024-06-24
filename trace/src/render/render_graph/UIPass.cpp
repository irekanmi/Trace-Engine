#include "pch.h"

#include "UIPass.h"
#include "render/Renderer.h"
#include "RenderGraph.h"
#include "backends/Renderutils.h"
#include "render/GPipeline.h"
#include "resource/ResourceSystem.h"
#include "FrameData.h"
#include "render/ShaderParser.h"
#include "render/GShader.h"
#include "backends/UIutils.h"


namespace trace {



	void UIPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R8G8B8A8_UNORM;
			color_attach.initial_format = TextureFormat::SHADER_READ;
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
			m_renderer->GetAvaliableRenderPasses()["UI_PASS"] = &m_renderPass;
		};


		/*{
			Ref<GShader> VertShader = ResourceSystem::get_instance()->CreateShader("fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = ResourceSystem::get_instance()->CreateShader("ui.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("UI_PASS");
			_ds2.depth_sten_state = { false, false };
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


			if (!ResourceSystem::get_instance()->CreatePipeline(_ds2, "ui_pass_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create ui_pass_pipeline");
				return;
			}
			m_pipeline = ResourceSystem::get_instance()->GetPipeline("ui_pass_pipeline");

		};*/

	}

	void UIPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void UIPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{

		FrameData& frame_data = black_board.get<FrameData>();

		auto pass = render_graph->AddPass("UI_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentInput(render_graph->GetResource(frame_data.ldr_index).resource_name);
		pass->AddColorAttachmentOuput(render_graph->GetResource(frame_data.swapchain_index).resource_name);



		pass->SetRunCB([=](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);

				UIFunc::UIRenderFrame(m_renderer);

			});

	}

	void UIPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}