#include "pch.h"

#include "ShadowPass.h"
#include "render/Renderer.h"
#include "RenderGraph.h"
#include "backends/Renderutils.h"
#include "render/GPipeline.h"

#include "resource/GenericAssetManager.h"
#include "FrameData.h"
#include "render/ShaderParser.h"
#include "render/GShader.h"
#include "glm/glm.hpp"
#include "render/Model.h"
#include "render/Camera.h"
#include "render/SkinnedModel.h"


namespace trace {

	extern UUID GetUUIDFromName(const std::string& name);

	void ShadowPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 0;
			depth_attach.attachment_format = Format::D32_SFLOAT;
			depth_attach.initial_format = TextureFormat::UNKNOWN;
			depth_attach.final_format = TextureFormat::DEPTH;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo att_infos[] = {
				depth_attach
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
			m_renderer->GetAvaliableRenderPasses()["SUN_SHADOW_MAP_PASS"] = &m_renderPass;
			m_renderer->GetAvaliableRenderPasses()["SPOT_SHADOW_MAP_PASS"] = &m_renderPass;
		};

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		if (AppSettings::is_editor)
		{
			Ref<GShader> VertShader = asset_manager->CreateAssetHandle<GShader>("sun_shadow.vert.glsl", "sun_shadow.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = asset_manager->CreateAssetHandle<GShader>("empty.frag.glsl", "empty.frag.glsl", ShaderStage::PIXEL_SHADER);

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(VertShader.get(), s_res);
			ShaderParser::generate_shader_resources(FragShader.get(), s_res);

			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = VertShader.get();
			_ds2.pixel_shader = FragShader.get();
			_ds2.resources = s_res;


			AutoFillPipelineDesc(
				_ds2
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("SUN_SHADOW_MAP_PASS");
			_ds2.depth_sten_state = { true, true, false, 0.0f, 1.0f };
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


			m_sun_pipeline = asset_manager->CreateAssetHandle<GPipeline>("sun_shadow_map_pass_pipeline", _ds2);
			if (!m_sun_pipeline)
			{
				TRC_ERROR("Failed to initialize or create sun_shadow_map_pass_pipeline");
				return;
			}

			// Skinned Models -------------------

			VertShader = asset_manager->CreateAssetHandle<GShader>("sun_shadow_skinned.vert.glsl", "sun_shadow_skinned.vert.glsl", ShaderStage::VERTEX_SHADER);

			ShaderResources sun_skinned_res = {};
			ShaderParser::generate_shader_resources(VertShader.get(), sun_skinned_res);
			ShaderParser::generate_shader_resources(FragShader.get(), sun_skinned_res);

			_ds2.resources = sun_skinned_res;
			_ds2.vertex_shader = VertShader.get();
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };
			_ds2.input_layout = SkinnedVertex::get_input_layout();

			m_sun_skinned_pipeline = asset_manager->CreateAssetHandle<GPipeline>("sun_shadow_skinned_map_pass_pipeline", _ds2);
			if (!m_sun_skinned_pipeline)
			{
				TRC_ERROR("Failed to initialize or create sun_shadow_skinned_map_pass_pipeline");
				return;
			}

			_ds2.rasteriser_state = { CullMode::BACK, FillMode::SOLID };

			m_spot_skinned_pipeline = asset_manager->CreateAssetHandle<GPipeline>("spot_shadow_skinned_map_pass_pipeline", _ds2);
			if (!m_spot_skinned_pipeline)
			{
				TRC_ERROR("Failed to initialize or create spot_shadow_skinned_map_pass_pipeline");
				return;
			}

			// -----------------------------------

			VertShader = asset_manager->CreateAssetHandle<GShader>("spot_shadow.vert.glsl", "spot_shadow.vert.glsl", ShaderStage::VERTEX_SHADER);

			ShaderResources spot_res = {};
			ShaderParser::generate_shader_resources(VertShader.get(), spot_res);
			ShaderParser::generate_shader_resources(FragShader.get(), spot_res);

			_ds2.resources = spot_res;
			_ds2.vertex_shader = VertShader.get();
			_ds2.rasteriser_state = { CullMode::BACK, FillMode::SOLID };
			_ds2.input_layout = Vertex::get_input_layout();

			m_spot_pipeline = asset_manager->CreateAssetHandle<GPipeline>("spot_shadow_map_pass_pipeline", _ds2);
			if (!m_spot_pipeline)
			{
				TRC_ERROR("Failed to initialize or create spot_shadow_map_pass_pipeline");
				return;
			}

		}
		else
		{
			UUID id = GetUUIDFromName("sun_shadow_map_pass_pipeline");
			m_sun_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

			id = GetUUIDFromName("spot_shadow_map_pass_pipeline");
			m_spot_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

			id = GetUUIDFromName("sun_shadow_skinned_map_pass_pipeline");
			m_sun_skinned_pipeline = asset_manager->Load_Runtime<GPipeline>(id);

			id = GetUUIDFromName("spot_shadow_skinned_map_pass_pipeline");
			m_spot_skinned_pipeline = asset_manager->Load_Runtime<GPipeline>(id);
		}

	}

	void ShadowPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void ShadowPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index)
	{
		Renderer* renderer = m_renderer;
		GPipeline* sun_shadow_map_pipeline = m_sun_pipeline.get();
		GPipeline* sun_skinned_pipeline = m_sun_skinned_pipeline.get();
		GPipeline* spot_skinned_pipeline = m_spot_skinned_pipeline.get();
		GPipeline* spot_shadow_map_pipeline = m_spot_pipeline.get();

		RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);


		int32_t current_resource_index = static_cast<int32_t>(render_graph->GetResources().size());

		ShadowMapData& shadow_map_data = black_board.add<ShadowMapData>();
		shadow_map_data.start_texture_index = current_resource_index;

		uint32_t sun_map_width = 1280;// TODO: Configurable
		uint32_t sun_map_height = 1280;//TODO: Configurable

		TextureDesc depth = {};
		depth.m_addressModeU = depth.m_addressModeV = depth.m_addressModeW = AddressMode::CLAMP_TO_BORDER;
		depth.m_attachmentType = AttachmentType::DEPTH;
		depth.m_flag = BindFlag::DEPTH_STENCIL_BIT;
		depth.m_format = Format::D32_SFLOAT;
		depth.m_width = sun_map_width;
		depth.m_height = sun_map_height;
		depth.m_minFilterMode = depth.m_magFilterMode = FilterMode::LINEAR;
		depth.m_mipLevels = depth.m_numLayers = 1;
		depth.m_usage = UsageFlag::DEFAULT;

		for (uint32_t i = 0; i < graph_data->num_shadowed_sun_lights; i++)
		{
			std::string pass_name = "SUN_SHADOW_MAP_PASS" + std::to_string(i);
			auto sun_pass = render_graph->AddPass(pass_name, GPU_QUEUE::GRAPHICS);
			uint32_t current_light_index = i;
			
			sun_pass->CreateDepthAttachmentOutput(pass_name, depth);
			
			sun_pass->SetRunCB([sun_map_width, sun_map_height, sun_shadow_map_pipeline, current_light_index, &shadow_map_data, sun_skinned_pipeline, draw_index](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
				{
					RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);
					Light& current_light = graph_data->sun_lights[current_light_index];

					Viewport view_port = renderer->_viewPort;
					Rect2D rect = renderer->_rect;
					view_port.width = static_cast<float>(sun_map_width);
					view_port.height = static_cast<float>(sun_map_height);

					rect.right = sun_map_width;
					rect.bottom = sun_map_height;


					Camera cam;
					cam.SetOrthographicSize(100.0f);
					cam.SetNear(-1000.0f);
					cam.SetFar(500.0f);
					cam.SetCameraType(CameraType::ORTHOGRAPHIC);
					cam.SetPosition(glm::vec3(0.0f));
					cam.SetLookDir(glm::vec3(current_light.direction));
					cam.SetScreenWidth((float)sun_map_width);
					cam.SetScreenHeight((float)sun_map_width);
					
					glm::mat4 view = cam.GetViewMatrix();
					glm::mat4 projection = cam.GetProjectionMatix();
					glm::mat4 view_proj = projection * view;
					
					shadow_map_data.sun_view_proj_matrices[current_light_index] = view_proj;


					for(RenderObjectData& object : graph_data->shadow_casters)
					{
						glm::mat4& model = object.transform;
						Model* _model = object.object;
						RenderFunc::OnDrawStart(renderer->GetDevice(), sun_shadow_map_pipeline);

						RenderFunc::BindViewport(renderer->GetDevice(), view_port);
						RenderFunc::BindRect(renderer->GetDevice(), rect);

						RenderFunc::SetPipelineData(sun_shadow_map_pipeline, "_view_proj", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_proj[0].x, sizeof(glm::mat4), render_graph_index);
						RenderFunc::SetPipelineData(sun_shadow_map_pipeline, "_model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, &model[0].x, sizeof(glm::mat4), render_graph_index);


						RenderFunc::BindPipeline_(sun_shadow_map_pipeline, render_graph_index);
						RenderFunc::BindPipeline(renderer->GetDevice(), sun_shadow_map_pipeline);

						RenderFunc::BindVertexBuffer(renderer->GetDevice(), _model->GetVertexBuffer());
						RenderFunc::BindIndexBuffer(renderer->GetDevice(), _model->GetIndexBuffer());

						RenderFunc::DrawIndexed(renderer->GetDevice(), 0, _model->GetIndexCount());

						RenderFunc::OnDrawEnd(renderer->GetDevice(), sun_shadow_map_pipeline);
					}


					for (RenderSkinnedObjectData& data : graph_data->skinned_shadow_casters)
					{

						glm::mat4* M_model = &data.transform;
						SkinnedModel* _model = data.object;
						if (!data.material)
						{
							continue;
						}

						RenderFunc::BindViewport(renderer->GetDevice(), view_port);
						RenderFunc::BindRect(renderer->GetDevice(), rect);

						RenderFunc::OnDrawStart(renderer->GetDevice(), sun_skinned_pipeline);
						RenderFunc::SetPipelineData(sun_skinned_pipeline, "_view_proj", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_proj[0].x, sizeof(glm::mat4), render_graph_index);
						RenderFunc::SetPipelineData(sun_skinned_pipeline, "_bone_matrices", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, data.bone_transforms, sizeof(glm::mat4) * data.bone_count, render_graph_index);
						RenderFunc::BindPipeline(renderer->GetDevice(), sun_skinned_pipeline);
						RenderFunc::BindPipeline_(sun_skinned_pipeline, render_graph_index);
						RenderFunc::BindVertexBuffer(renderer->GetDevice(), _model->GetVertexBuffer());
						RenderFunc::BindIndexBuffer(renderer->GetDevice(), _model->GetIndexBuffer());

						RenderFunc::DrawIndexed(renderer->GetDevice(), 0, _model->GetIndexCount());
						RenderFunc::OnDrawEnd(renderer->GetDevice(), sun_skinned_pipeline);
					}

					


				});

			shadow_map_data.total_shadowed_lights++;
		}
		

		// SpotLights
		uint32_t spot_map_width = 768;// TODO: Configurable
		uint32_t spot_map_height = 768;//TODO: Configurable

		depth.m_width = spot_map_width;
		depth.m_height = spot_map_height;


		for (uint32_t i = 0; i < graph_data->num_shadowed_spot_lights; i++)
		{
			std::string spot_pass_name = "SPOT_SHADOW_MAP_PASS" + std::to_string(i);
			//uint32_t res_index = render_graph->AddTextureResource(pass_name, depth);
			auto spot_pass = render_graph->AddPass(spot_pass_name, GPU_QUEUE::GRAPHICS);
			uint32_t current_light_index = i;

			spot_pass->CreateDepthAttachmentOutput(spot_pass_name, depth);

			spot_pass->SetRunCB([spot_map_width, spot_map_height, spot_shadow_map_pipeline, current_light_index, &shadow_map_data, spot_skinned_pipeline, draw_index](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
				{
					RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(draw_index);
					Light& current_light = graph_data->spot_lights[current_light_index];


					Viewport view_port = renderer->_viewPort;
					Rect2D rect = renderer->_rect;
					view_port.width = static_cast<float>(spot_map_width);
					view_port.height = static_cast<float>(spot_map_height);

					rect.right = spot_map_width;
					rect.bottom = spot_map_height;

					Camera cam;
					cam.SetNear(1.0f);
					cam.SetFar(250.0f);
					cam.SetCameraType(CameraType::PERSPECTIVE);
					cam.SetPosition(glm::vec3(current_light.position));
					cam.SetLookDir(glm::vec3(current_light.direction));
					cam.SetScreenWidth((float)spot_map_width);
					cam.SetScreenHeight((float)spot_map_height);

					float fov = glm::degrees(glm::acos(current_light.params2.x)) * 2.0f;
					cam.SetFov(fov);

					glm::mat4 view = cam.GetViewMatrix();
					glm::mat4 projection = cam.GetProjectionMatix();
					glm::mat4 view_proj = projection * view;

					shadow_map_data.spot_view_proj_matrices[current_light_index] = view_proj;

					for (RenderObjectData& object : graph_data->shadow_casters)
					{
						glm::mat4& model = object.transform;
						Model* _model = object.object;

						RenderFunc::OnDrawStart(renderer->GetDevice(), spot_shadow_map_pipeline);
						RenderFunc::BindViewport(renderer->GetDevice(), view_port);
						RenderFunc::BindRect(renderer->GetDevice(), rect);

						RenderFunc::SetPipelineData(spot_shadow_map_pipeline, "_view_proj", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_proj[0].x, sizeof(glm::mat4), render_graph_index);
						RenderFunc::SetPipelineData(spot_shadow_map_pipeline, "_model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, &model[0].x, sizeof(glm::mat4), render_graph_index);


						RenderFunc::BindPipeline_(spot_shadow_map_pipeline, render_graph_index);
						RenderFunc::BindPipeline(renderer->GetDevice(), spot_shadow_map_pipeline);

						RenderFunc::BindVertexBuffer(renderer->GetDevice(), _model->GetVertexBuffer());
						RenderFunc::BindIndexBuffer(renderer->GetDevice(), _model->GetIndexBuffer());

						RenderFunc::DrawIndexed(renderer->GetDevice(), 0, _model->GetIndexCount());
						RenderFunc::OnDrawEnd(renderer->GetDevice(), spot_shadow_map_pipeline);
					}

					for (RenderSkinnedObjectData& data : graph_data->skinned_shadow_casters)
					{

						glm::mat4* M_model = &data.transform;
						SkinnedModel* _model = data.object;
						if (!data.material)
						{
							continue;
						}

						RenderFunc::BindViewport(renderer->GetDevice(), view_port);
						RenderFunc::BindRect(renderer->GetDevice(), rect);

						RenderFunc::OnDrawStart(renderer->GetDevice(), spot_skinned_pipeline);
						RenderFunc::SetPipelineData(spot_skinned_pipeline, "_view_proj", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_proj[0].x, sizeof(glm::mat4), render_graph_index);
						RenderFunc::SetPipelineData(spot_skinned_pipeline, "_bone_matrices", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, data.bone_transforms, sizeof(glm::mat4) * data.bone_count, render_graph_index);
						RenderFunc::BindPipeline(renderer->GetDevice(), spot_skinned_pipeline);
						RenderFunc::BindPipeline_(spot_skinned_pipeline, render_graph_index);
						RenderFunc::BindVertexBuffer(renderer->GetDevice(), _model->GetVertexBuffer());
						RenderFunc::BindIndexBuffer(renderer->GetDevice(), _model->GetIndexBuffer());

						RenderFunc::DrawIndexed(renderer->GetDevice(), 0, _model->GetIndexCount());
						RenderFunc::OnDrawEnd(renderer->GetDevice(), spot_skinned_pipeline);
					}

				});
			
			shadow_map_data.total_shadowed_lights++;
		}

	}

	void ShadowPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}