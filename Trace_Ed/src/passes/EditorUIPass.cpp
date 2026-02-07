
#include <trace.h>
#include <render/Renderer.h>
#include <render/render_graph/RenderGraph.h>
#include <backends/Renderutils.h>
#include <render/GPipeline.h>
#include <resource/ResourceSystem.h>
#include <render/render_graph/FrameData.h>
#include <render/ShaderParser.h>
#include <render/GShader.h>
#include <backends/UIutils.h>
#include <resource/GenericAssetManager.h>
#include "EditorUIPass.h"
#include "../TraceEditor.h"


namespace trace {



	void EditorUIPass::Init(Renderer* renderer)
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


			AttachmentInfo att_infos[2] = {
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
			
			color_attach.attachment_format = Format::R32G32B32A32_FLOAT;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;

			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 1;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			att_infos[0] = color_attach;
			att_infos[1] = depth_attach;

			subpass_desc;
			subpass_desc.attachment_count = 2;
			subpass_desc.attachments = att_infos;

			pass_desc.subpass = subpass_desc;

			RenderFunc::CreateRenderPass(&m_object_pick_renderpass, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["OBJECT_PICK_PASS"] = &m_object_pick_renderpass;


		};

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		Ref<GShader> VertShader = asset_manager->CreateAssetHandle<GShader>("trace_core.shader.vert.glsl", "trace_core.shader.vert.glsl", ShaderStage::VERTEX_SHADER);
		Ref<GShader> FragShader = asset_manager->CreateAssetHandle<GShader>("object_pick.frag.glsl", "object_pick.frag.glsl", ShaderStage::PIXEL_SHADER);

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
		_ds2.render_pass = Renderer::get_instance()->GetRenderPass("OBJECT_PICK_PASS");
		_ds2.depth_sten_state = { true, false, false, 0.0f, 1.0f };
		_ds2.rasteriser_state = { CullMode::BACK, FillMode::SOLID };


		m_object_pick_pipeline = asset_manager->CreateAssetHandle<GPipeline>("object_pick_pipeline", _ds2);
		if (!m_object_pick_pipeline)
		{
			TRC_ERROR("Failed to initialize or create object_pick_pipeline");
			return;
		}

	}

	void EditorUIPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void EditorUIPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		RenderComposer* render_composer = Renderer::get_instance()->GetRenderComposer();


		FrameData& frame_data = black_board.get<FrameData>();
		std::vector<RenderGraphInfo>& graphs = render_composer->GetGraphs();

		bool add_object_pick_pass = (render_graph_index == 0) && (graphs.size() > 1) && (graphs[1].built) && editor->should_select;
		uint32_t object_pick_render_target = INVALID_ID;

		if (add_object_pick_pass)
		{

			auto object_pick_pass = render_graph->AddPass("OBJECT_PICK_PASS", GPU_QUEUE::GRAPHICS);

			FrameData& scene_frame_data = graphs[1].black_board.get<FrameData>();
			GBufferData& gbuffer_data = graphs[1].black_board.get<GBufferData>();
			uint32_t depth_index = render_graph->AddTextureResource("GRAPH_DEPTH_BUFFER", &graphs[1].graph, gbuffer_data.depth_index);

			TextureDesc object_id_map = {};
			object_id_map.m_addressModeU = object_id_map.m_addressModeV = object_id_map.m_addressModeW = AddressMode::CLAMP_TO_BORDER;
			object_id_map.m_attachmentType = AttachmentType::COLOR;
			object_id_map.m_flag = BindFlag::RENDER_TARGET_BIT;
			object_id_map.m_format = Format::R32G32B32A32_FLOAT;
			object_id_map.m_width = scene_frame_data.frame_width;
			object_id_map.m_height = scene_frame_data.frame_height;
			object_id_map.m_minFilterMode = object_id_map.m_magFilterMode = FilterMode::LINEAR;
			object_id_map.m_mipLevels = object_id_map.m_numLayers = 1;
			object_id_map.m_usage = UsageFlag::DEFAULT;

			object_pick_render_target = object_pick_pass->CreateAttachmentOutput("out_id", object_id_map);
			object_pick_pass->SetDepthStencilInput(depth_index);

			Ref<GPipeline> object_pick_pipeline = m_object_pick_pipeline;

			object_pick_pass->SetRunCB([scene_frame_data, object_pick_pipeline](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
				{
					Viewport view_port = renderer->_viewPort;
					Rect2D rect = renderer->_rect;
					view_port.width = static_cast<float>(scene_frame_data.frame_width);
					view_port.height = static_cast<float>(scene_frame_data.frame_height);

					rect.right = scene_frame_data.frame_width;
					rect.bottom = scene_frame_data.frame_height;

					RenderFunc::BindViewport(renderer->GetDevice(), view_port);
					RenderFunc::BindRect(renderer->GetDevice(), rect);

					//Render all objects
					RenderGraphFrameData& graph_data = *renderer->GetRenderGraphData(1);

					std::vector<RenderObjectData>& m_opaqueObjects = graph_data.m_opaqueObjects;
					uint32_t& m_opaqueObjectsSize = graph_data.m_opaqueObjectsSize;
					Camera* _camera = graph_data._camera;

					if (!_camera)
					{
						return;
					}

					glm::mat4 proj = _camera->GetProjectionMatix();
					glm::mat4 view = _camera->GetViewMatrix();
					glm::vec3 view_position = _camera->GetPosition();
					glm::mat4 view_proj = proj * view;

					for (RenderObjectData& data : m_opaqueObjects)
					{

						glm::mat4* M_model = &data.transform;
						Model* _model = data.object;
						Ref<GPipeline> sp = object_pick_pipeline;
						glm::vec4 object_id(0.0f);
						memcpy(&object_id, &data.object_id, sizeof(UUID));

						RenderFunc::OnDrawStart(renderer->GetDevice(), sp.get());
						RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
						RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4), 0, render_graph_index);
						RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3), 0, render_graph_index);
						RenderFunc::SetPipelineData(sp.get(), "_model", ShaderResourceStage::RESOURCE_STAGE_DRAW_CALL, M_model, sizeof(glm::mat4), 0, render_graph_index);
						RenderFunc::SetPipelineData(sp.get(), "object_id", ShaderResourceStage::RESOURCE_STAGE_DRAW_CALL, &object_id, sizeof(glm::vec4), 0, render_graph_index);
						RenderFunc::BindPipeline(renderer->GetDevice(), sp.get());
						RenderFunc::BindPipeline_(sp.get(), render_graph_index);
						RenderFunc::BindVertexBuffer(renderer->GetDevice(), _model->GetVertexBuffer());
						RenderFunc::BindIndexBuffer(renderer->GetDevice(), _model->GetIndexBuffer());

						RenderFunc::DrawIndexed(renderer->GetDevice(), 0, _model->GetIndexCount());
						RenderFunc::OnDrawEnd(renderer->GetDevice(), sp.get());
					}


				});

			object_pick_pass->SetPassEndCB([object_pick_render_target, editor](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
				{
					void* p_data = &editor->pixel_data;
					
					glm::ivec3 offset(editor->select_pos.x, editor->select_pos.y, 0);
					glm::uvec3 extent(1, 1, 1);	


					RenderGraphResource* resource = render_graph->GetResource_ptr(object_pick_render_target);
					RenderFunc::GetRenderGraphTextureData(render_graph, resource, offset, extent, p_data);

					

				});

			editor->should_select = false;

		}

		auto pass = render_graph->AddPass("EDITOR_UI_PASS", GPU_QUEUE::GRAPHICS);

		//pass->AddColorAttachmentInput(render_graph->GetResource(frame_data.ldr_index).resource_name);
		if (add_object_pick_pass)
		{
			pass->AddColorAttachmentInput(object_pick_render_target);
		}
		pass->AddColorAttachmentOuput(render_graph->GetResource(frame_data.swapchain_index).resource_name);

		if (render_graph_index == 0)
		{		
			
			for(int32_t i = 1; i < graphs.size(); i++)
			{
				if (!graphs[i].built)
				{
					continue;
				}
				RenderGraph* _graph = render_composer->GetRenderGraph(i);
				pass->AddTextureInput("Index_Tex" + std::to_string(i), _graph, _graph->GetFinalResourceOutput());
			}
		}

		static std::vector<void*> texture_handles(MAX_RENDER_GRAPH);
		std::vector<void*>* tex_handle = &texture_handles;

		pass->SetRunCB([frame_data, tex_handle](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(renderer->GetDevice(), renderer->_viewPort);
				RenderFunc::BindRect(renderer->GetDevice(), renderer->_rect);

				RenderComposer* render_composer = renderer->GetRenderComposer();
				std::vector<RenderGraphInfo>& graphs = render_composer->GetGraphs();

				for (int32_t i = 1; i < graphs.size(); i++)
				{
					if (!graphs[i].built)
					{
						continue;
					}
					RenderGraph* _graph = render_composer->GetRenderGraph(i);
					UIFunc::GetDrawRenderGraphTextureHandle(_graph->GetResource_ptr(_graph->GetFinalResourceOutput()), (*tex_handle)[i]);
					
				}

				
				
				TraceEditor::get_instance()->RenderViewport(*tex_handle);

				UIFunc::UIRenderFrame(renderer);

			});


	}

	void EditorUIPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
		RenderFunc::DestroyRenderPass(&m_object_pick_renderpass);
	}

}