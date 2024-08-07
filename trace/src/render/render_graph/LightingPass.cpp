#include "pch.h"

#include "LightingPass.h"
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


	void LightingPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R16G16B16A16_FLOAT;
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
			m_renderer->GetAvaliableRenderPasses()["LIGHTING_PASS"] = &m_renderPass;
		};

		{
			TextureDesc color = {};
			color.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
			color.m_attachmentType = AttachmentType::COLOR;
			color.m_flag = BindFlag::RENDER_TARGET_BIT;
			color.m_format = Format::R16G16B16A16_FLOAT;
			color.m_width = 800;
			color.m_height = 600;
			color.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
			color.m_mipLevels = color.m_numLayers = 1;
			color.m_usage = UsageFlag::DEFAULT;

			output_desc = color;
		}

		if(AppSettings::is_editor)
		{
			Ref<GShader> VertShader = ShaderManager::get_instance()->CreateShader("fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = ShaderManager::get_instance()->CreateShader("lighting.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("LIGHTING_PASS");
			_ds2.depth_sten_state = { false, false };
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


			m_pipeline = PipelineManager::get_instance()->CreatePipeline(_ds2, "lighting_pass_pipeline");
			if (!m_pipeline)
			{
				TRC_ERROR("Failed to initialize or create lighting_pass_pipeline");
				return;
			}


		}
		else
		{
			UUID id = GetUUIDFromName("lighting_pass_pipeline");
			m_pipeline = PipelineManager::get_instance()->LoadPipeline_Runtime(id);
		}

	}

	void LightingPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{

	}

	void LightingPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		FrameData& frame_data = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();
		output_desc.m_width = frame_data.frame_width;
		output_desc.m_height = frame_data.frame_height;


		gPosition_index = gbuffer_data.position_index;
		gNormal_index = gbuffer_data.normal_index;
		gColor_index = gbuffer_data.color_index;

		SSAOData* ssao = black_board.try_get<SSAOData>();
		bool ssao_avaliable = false;

		auto pass = render_graph->AddPass("LIGHTING_PASS", GPU_QUEUE::GRAPHICS);

		color_output_index = pass->CreateAttachmentOutput("Hdr_Target", output_desc);
		frame_data.hdr_index = color_output_index;

		pass->AddColorAttachmentInput(render_graph->GetResource(gPosition_index).resource_name);
		pass->AddColorAttachmentInput(render_graph->GetResource(gNormal_index).resource_name);
		pass->AddColorAttachmentInput(render_graph->GetResource(gColor_index).resource_name);

		if (ssao)
		{
			pass->AddColorAttachmentInput(render_graph->GetResource(ssao->ssao_blur).resource_name);
			ssao_avaliable = true;
		}
		
		uint32_t width = render_graph->GetResource(frame_data.hdr_index).resource_data.texture.width;
		uint32_t height = render_graph->GetResource(frame_data.hdr_index).resource_data.texture.height;

		pass->SetRunCB([=](std::vector<uint32_t>& inputs)
			{
				Viewport view_port = m_renderer->_viewPort;
				Rect2D rect = m_renderer->_rect;
				view_port.width = width;
				view_port.height = height;

				rect.right = width;
				rect.bottom = height;

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_pipeline.get());

				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);


				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"g_bufferData0",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(gPosition_index),
					0
				);

				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"g_bufferData1",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(gNormal_index),
					1
				);

				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"g_bufferData2",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(gColor_index),
					2
				);
				uint32_t res = 0;
				uint32_t ssao_index = INVALID_ID;
				if (ssao_avaliable)
				{
					res = 1;
					ssao_index = ssao->ssao_blur;
				}
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"ssao_blur",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(ssao_index)
				);

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"_ssao_dat",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&res,
					sizeof(uint32_t)
				);


				/*RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"u_gLights",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					m_renderer->lights.data(),
					sizeof(Light) * MAX_LIGHT_COUNT
				);*/

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"light_positions",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					m_renderer->light_positions.data(),
					sizeof(glm::vec4) * m_renderer->light_positions.size()
				);

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"light_directions",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					m_renderer->light_directions.data(),
					sizeof(glm::vec4) * m_renderer->light_directions.size()
				);

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"light_colors",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					m_renderer->light_colors.data(),
					sizeof(glm::vec4) * m_renderer->light_colors.size()
				);

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"light_params1s",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					m_renderer->light_params1s.data(),
					sizeof(glm::vec4)* m_renderer->light_params1s.size()
				); 
				
				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"light_params2s",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					m_renderer->light_params2s.data(),
					sizeof(glm::vec4) * m_renderer->light_params2s.size()
				);

				glm::mat4 view = m_renderer->_camera->GetViewMatrix();

				RenderFunc::SetPipelineData(
					m_pipeline.get(),
					"_view",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&view,
					sizeof(glm::mat4)
				);

				RenderFunc::SetPipelineData(m_pipeline.get(), "_light_data", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &m_renderer->light_data, sizeof(glm::ivec4));
				frame_count++;

				RenderFunc::BindPipeline_(m_pipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);

				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_pipeline.get());


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