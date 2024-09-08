#include "pch.h"

#include "ToneMapPass.h"
#include "render/Renderer.h"
#include "RenderGraph.h"
#include "backends/Renderutils.h"
#include "render/GPipeline.h"
#include "resource/PipelineManager.h"
#include "resource/ShaderManager.h"
#include "FrameData.h"
#include "render/ShaderParser.h"
#include "render/GShader.h"


namespace trace {

	extern UUID GetUUIDFromName(const std::string& name);

	void ToneMapPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R8G8B8A8_UNORM;
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
			m_renderer->GetAvaliableRenderPasses()["TONEMAP_PASS"] = &m_renderPass;
		};

		if(AppSettings::is_editor)
		{
			Ref<GShader> VertShader = ShaderManager::get_instance()->CreateShader("fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = ShaderManager::get_instance()->CreateShader("tone_map.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("TONEMAP_PASS");
			_ds2.depth_sten_state = { false, false };
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


			m_pipeline = PipelineManager::get_instance()->CreatePipeline(_ds2, "tone_map_pass_pipeline");
			if (!m_pipeline)
			{
				TRC_ERROR("Failed to initialize or create tone_map_pass_pipeline");
				return;
			}

		}
		else
		{
			UUID id = GetUUIDFromName("tone_map_pass_pipeline");
			m_pipeline = PipelineManager::get_instance()->LoadPipeline_Runtime(id);
		}

	}

	void ToneMapPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void ToneMapPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index)
	{

		FrameData& frame_data = black_board.get<FrameData>();
		
		auto pass = render_graph->AddPass("TONEMAP_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentOuput(frame_data.ldr_index);

		pass->AddColorAttachmentInput(frame_data.hdr_index);
		uint32_t width = render_graph->GetResource(frame_data.ldr_index).resource_data.texture.width;
		uint32_t height = render_graph->GetResource(frame_data.ldr_index).resource_data.texture.height;

		pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);
				Viewport view_port = m_renderer->_viewPort;
				Rect2D rect = m_renderer->_rect;
				view_port.width = width;
				view_port.height = height;

				rect.right = width;
				rect.bottom = height;
				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_pipeline.get());
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"u_HdrTarget",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(frame_data.hdr_index)
				);

				float exposure = m_renderer->exposure;
				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"exposure",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&exposure,
					sizeof(float)
				);

				RenderFunc::BindPipeline_(m_pipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_pipeline.get());


			});

	}

	void ToneMapPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}