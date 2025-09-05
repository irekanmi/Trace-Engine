#include "pch.h"

#include "LightingPass.h"
#include "render/Renderer.h"
#include "RenderGraph.h"
#include "backends/Renderutils.h"
#include "render/GPipeline.h"

#include "resource/GenericAssetManager.h"
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

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		if(AppSettings::is_editor)
		{
			Ref<GShader> VertShader;
			Ref<GShader> FragShader;

			Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

			Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("lighting.frag.glsl", "lighting.frag.glsl", ShaderStage::PIXEL_SHADER);

			VertShader = vert_shader;
			FragShader = frag_shader;

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


			m_pipeline = asset_manager->CreateAssetHandle<GPipeline>("lighting_pass_pipeline", _ds2);
			if (!m_pipeline)
			{
				TRC_ERROR("Failed to initialize or create lighting_pass_pipeline");
				return;
			}


		}
		else
		{
			UUID id = GetUUIDFromName("lighting_pass_pipeline");
			m_pipeline = asset_manager->Load_Runtime<GPipeline>(id);
		}

	}

	void LightingPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{

	}

	void LightingPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index)
	{
		FrameData& frame_data = black_board.get<FrameData>();
		GBufferData* gbuffer_data = black_board.try_get<GBufferData>();
		output_desc.m_width = frame_data.frame_width;
		output_desc.m_height = frame_data.frame_height;



		SSAOData* ssao = black_board.try_get<SSAOData>();
		bool ssao_avaliable = false;

		auto pass = render_graph->AddPass("LIGHTING_PASS", GPU_QUEUE::GRAPHICS);

		color_output_index = pass->CreateAttachmentOutput("Hdr_Target", output_desc);
		frame_data.hdr_index = color_output_index;

		pass->AddColorAttachmentInput(gbuffer_data->position_index);
		pass->AddColorAttachmentInput(gbuffer_data->normal_index);
		pass->AddColorAttachmentInput(gbuffer_data->color_index);
		pass->AddColorAttachmentInput(gbuffer_data->emissive_index);

		if (ssao)
		{
			pass->AddColorAttachmentInput(render_graph->GetResource(ssao->ssao_blur).resource_name);
			ssao_avaliable = true;
		}
		
		uint32_t width = render_graph->GetResource(frame_data.hdr_index).resource_data.texture.width;
		uint32_t height = render_graph->GetResource(frame_data.hdr_index).resource_data.texture.height;

		//Shadows .............................................
		ShadowMapData* shadow_data = black_board.try_get<ShadowMapData>();

		if (shadow_data)
		{
			for (uint32_t i = 0; i < shadow_data->total_shadowed_lights; i++)
			{
				uint32_t index = shadow_data->start_texture_index + i;

				pass->AddColorAttachmentInput(index);
			}
		}
		// ....................................................

		GPipeline* pipeline = m_pipeline.get();
		pass->SetRunCB([width, height, pipeline, ssao, ssao_avaliable, shadow_data, gbuffer_data](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);

				if (!graph_data->_camera)
				{
					return;
				}

				Viewport view_port = renderer->_viewPort;
				Rect2D rect = renderer->_rect;
				view_port.width = static_cast<float>(width);
				view_port.height = static_cast<float>(height);

				rect.right = width;
				rect.bottom = height;

				RenderFunc::OnDrawStart(renderer->GetDevice(), pipeline);

				RenderFunc::BindViewport(renderer->GetDevice(), view_port);
				RenderFunc::BindRect(renderer->GetDevice(), rect);


				RenderFunc::BindRenderGraphTexture( render_graph, pipeline, "g_bufferData", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(gbuffer_data->position_index), render_graph_index, 0 );

				RenderFunc::BindRenderGraphTexture( render_graph, pipeline, "g_bufferData", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(gbuffer_data->normal_index), render_graph_index, 1 );
				RenderFunc::BindRenderGraphTexture( render_graph, pipeline, "g_bufferData", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(gbuffer_data->emissive_index), render_graph_index, 2 );

				RenderFunc::BindRenderGraphTexture( render_graph, pipeline, "g_bufferColor", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(gbuffer_data->color_index), render_graph_index);
				uint32_t res = 0;
				uint32_t ssao_index = INVALID_ID;
				if (ssao_avaliable)
				{
					res = 1;
					ssao_index = ssao->ssao_blur;
				}
				RenderFunc::BindRenderGraphTexture( render_graph, pipeline, "ssao_blur", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(ssao_index), render_graph_index);

				RenderFunc::SetPipelineData( pipeline, "_ssao_dat", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &res, sizeof(uint32_t), render_graph_index);


				RenderFunc::SetPipelineData(pipeline, "num_shadowed_sun_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &graph_data->num_shadowed_sun_lights, sizeof(uint32_t), render_graph_index);
				RenderFunc::SetPipelineData(pipeline, "num_non_shadowed_sun_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &graph_data->num_non_shadowed_sun_lights, sizeof(uint32_t), render_graph_index);
				uint32_t total_sun_lights = graph_data->num_shadowed_sun_lights + graph_data->num_non_shadowed_sun_lights;
				RenderFunc::SetPipelineData(pipeline, "sun_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, graph_data->sun_lights.data(), sizeof(Light) * total_sun_lights, render_graph_index);

				RenderFunc::SetPipelineData(pipeline, "num_shadowed_spot_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, & graph_data->num_shadowed_spot_lights, sizeof(uint32_t), render_graph_index);
				RenderFunc::SetPipelineData(pipeline, "num_non_shadowed_spot_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &graph_data->num_non_shadowed_spot_lights, sizeof(uint32_t), render_graph_index);
				uint32_t total_spot_lights = graph_data->num_shadowed_spot_lights + graph_data->num_non_shadowed_spot_lights;
				RenderFunc::SetPipelineData(pipeline, "spot_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, graph_data->spot_lights.data(), sizeof(Light) * total_spot_lights, render_graph_index);

				RenderFunc::SetPipelineData(pipeline, "num_shadowed_point_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &graph_data->num_shadowed_point_lights, sizeof(uint32_t), render_graph_index);
				RenderFunc::SetPipelineData(pipeline, "num_non_shadowed_point_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &graph_data->num_non_shadowed_point_lights, sizeof(uint32_t), render_graph_index);
				uint32_t total_point_lights = graph_data->num_shadowed_point_lights + graph_data->num_non_shadowed_point_lights;
				RenderFunc::SetPipelineData(pipeline, "point_lights", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, graph_data->point_lights.data(), sizeof(Light) * total_point_lights, render_graph_index);

				
				// Shadow Maps --------------------------------------------

				if (shadow_data && shadow_data->total_shadowed_lights > 0)
				{
					RenderFunc::SetPipelineData(pipeline, "sun_shadow_view_proj", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, shadow_data->sun_view_proj_matrices.data(), sizeof(glm::mat4) * graph_data->num_shadowed_sun_lights, render_graph_index);
					for (uint32_t i = 0; i < graph_data->num_shadowed_sun_lights; i++)
					{
						uint32_t tex_index = shadow_data->start_texture_index + i;
						RenderFunc::BindRenderGraphTexture(render_graph, pipeline, "sun_shadow_maps", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(tex_index), render_graph_index, i);

					}


					RenderFunc::SetPipelineData(pipeline, "spot_shadow_view_proj", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, shadow_data->spot_view_proj_matrices.data(), sizeof(glm::mat4) * graph_data->num_shadowed_spot_lights, render_graph_index);
					for (uint32_t i = 0; i < graph_data->num_shadowed_spot_lights; i++)
					{
						uint32_t tex_index = shadow_data->start_texture_index + graph_data->num_shadowed_sun_lights + i;
						RenderFunc::BindRenderGraphTexture(render_graph, pipeline, "spot_shadow_maps", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(tex_index), render_graph_index, i);

					}

				}
				if (graph_data->num_shadowed_sun_lights <= 0)
				{
					//HACK: It crashes because the binded texture has been destroyed due to the use of frame graph
					for (uint32_t i = 0; i < MAX_SHADOW_SUN_LIGHTS; i++)
					{
						RenderFunc::BindRenderGraphTexture(render_graph, pipeline, "sun_shadow_maps", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, nullptr, render_graph_index, i);
					}
				}
				if (graph_data->num_shadowed_spot_lights <= 0)
				{
					//HACK: It crashes because the binded texture has been destroyed due to the use of frame graph
					for (uint32_t i = 0; i < MAX_SHADOW_SPOT_LIGHTS; i++)
					{
						RenderFunc::BindRenderGraphTexture(render_graph, pipeline, "spot_shadow_maps", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, nullptr, render_graph_index, i);
					}
				}

				// --------------------------------------------------------



				glm::mat4 view = graph_data->_camera->GetViewMatrix();
				glm::mat4 inv_view = glm::inverse(view);

				RenderFunc::SetPipelineData( pipeline, "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4), render_graph_index);
				RenderFunc::SetPipelineData( pipeline, "_inv_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &inv_view, sizeof(glm::mat4), render_graph_index);


				RenderFunc::BindPipeline_(pipeline, render_graph_index);
				RenderFunc::BindPipeline(renderer->GetDevice(), pipeline);
				RenderFunc::Draw(renderer->GetDevice(), 0, 3);

				RenderFunc::OnDrawEnd(renderer->GetDevice(), pipeline);


			});


	}

	void LightingPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}