#include "pch.h"

#include "ToneMapPass.h"
#include "render/Renderer.h"
#include "RenderGraph.h"
#include "render/Renderutils.h"
#include "render/GPipeline.h"
#include "resource/ResourceSystem.h"
#include "FrameData.h"
#include "render/ShaderParser.h"
#include "render/GShader.h"


namespace trace {



	void ToneMapPass::Init(Renderer* renderer)
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
			m_renderer->_avaliable_passes["TONEMAP_PASS"] = &m_renderPass;
		};


		{
			std::string vert_src;
			std::string frag_src;
			GShader VertShader;
			GShader FragShader;

			vert_src = ShaderParser::load_shader_file("../assets/shaders/fullscreen.vert.glsl");
			frag_src = ShaderParser::load_shader_file("../assets/shaders/tone_map.frag.glsl");

			RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
			RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(&VertShader, s_res);
			ShaderParser::generate_shader_resources(&FragShader, s_res);



			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = &VertShader;
			_ds2.pixel_shader = &FragShader;
			_ds2.resources = s_res;
			_ds2.input_layout = {};


			AutoFillPipelineDesc(
				_ds2,
				false
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("TONEMAP_PASS");
			_ds2.depth_sten_state = { false, false };


			if (!ResourceSystem::get_instance()->CreatePipeline(_ds2, "tone_map_pass_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create tone_map_pass_pipeline");
				RenderFunc::DestroyShader(&VertShader);
				RenderFunc::DestroyShader(&FragShader);
				return;
			}

			RenderFunc::DestroyShader(&VertShader);
			RenderFunc::DestroyShader(&FragShader);

			m_pipeline = ResourceSystem::get_instance()->GetPipeline("tone_map_pass_pipeline");

		};

	}

	void ToneMapPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void ToneMapPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{

		FrameData& frame_data = black_board.get<FrameData>();
		
		auto pass = render_graph->AddPass("TONEMAP_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentOuput(render_graph->GetResource(frame_data.ldr_index).resource_name);

		pass->AddColorAttachmentInput(render_graph->GetResource(frame_data.hdr_index).resource_name);


		pass->SetRunCB([=](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);


				RenderFunc::BindRenderGraphResource(
					render_graph,
					m_pipeline.get(),
					"u_HdrTarget",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&render_graph->GetResource(frame_data.hdr_index)
				);

				float exposure = m_renderer->exposure;
				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"exposure",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&exposure,
					sizeof(float)
				);

				RenderFunc::BindVertexBuffer(m_renderer->GetDevice(), &m_renderer->quadBuffer);
				RenderFunc::BindPipeline_(m_pipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);


			});

	}

	void ToneMapPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}