#include "ObjectHighlightPass.h"
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

	void ObjectHighlightPass::Init(Renderer* renderer)
	{

		m_renderer = renderer;

		{
			AttachmentInfo color_attach;
			color_attach.attachmant_index = 0;
			color_attach.attachment_format = Format::R8G8B8A8_UNORM;
			color_attach.initial_format = TextureFormat::SHADER_READ;
			color_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			color_attach.is_depth = false;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 1;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::SHADER_READ;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo att_infos[] = {
				color_attach,
				depth_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = ARRAYSIZE(att_infos);
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;


			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["OBJ_HIGHLIGHT_PASS"] = &m_renderPass;

			color_attach.attachment_format = Format::R8G8B8A8_UNORM;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			att_infos[0] = color_attach;

			subpass_desc.attachment_count = 1;
			subpass_desc.attachments = att_infos;

			pass_desc.subpass = subpass_desc;


			RenderFunc::CreateRenderPass(&imageDialationPass, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["IMAGE_DIALATION_PASS"] = &imageDialationPass;


			color_attach.attachment_format = Format::R8G8B8A8_UNORM;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			att_infos[0] = color_attach;

			subpass_desc.attachment_count = 2;
			subpass_desc.attachments = att_infos;

			pass_desc.subpass = subpass_desc;


			RenderFunc::CreateRenderPass(&finalBlendPass, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["FINAL_BLEND_PASS"] = &finalBlendPass;
		};

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		{
			Ref<GShader> VertShader;
			Ref<GShader> FragShader;

			Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("trace_core.shader.vert.glsl", "trace_core.shader.vert.glsl", ShaderStage::VERTEX_SHADER);

			Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("simple_color.frag.glsl", "simple_color.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("OBJ_HIGHLIGHT_PASS");

			DepthStencilState dp_state = {};
			dp_state.depth_test_enable = false;
			dp_state.depth_write_enable = false;
			dp_state.minDepth = 0.0f;
			dp_state.maxDepth = 1.0f;
			dp_state.stencil_test_enable = true;

			StencilState stencil_state = {};
			stencil_state.compareOp = CompareOp::COMPARE_ALWAYS;
			stencil_state.compare_mask = 0xff;
			stencil_state.depth_failOp = StencilOp::STENCIL_KEEP;
			stencil_state.failOp = StencilOp::STENCIL_KEEP;
			stencil_state.passOp = StencilOp::STENCIL_REPLACE;
			stencil_state.reference = 1;
			stencil_state.write_mask = 0xff;
			
			dp_state.stencil_state = stencil_state;

			_ds2.depth_sten_state = dp_state;
			_ds2.rasteriser_state = { CullMode::BACK, FillMode::SOLID };
			_ds2.input_layout = Vertex::get_input_layout();

			m_pipeline = asset_manager->CreateAssetHandle<GPipeline>("obj_highlight_pass_pipeline", _ds2);
			if (!m_pipeline)
			{
				TRC_ERROR("Failed to initialize or create obj_highlight_pass_pipeline");
				return;
			}
		};

		{
			Ref<GShader> VertShader;
			Ref<GShader> FragShader;

			Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

			Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("image_dialate.frag.glsl", "image_dialate.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("IMAGE_DIALATION_PASS");
			_ds2.depth_sten_state = { false, false };
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };

			imageDialationPipeline = asset_manager->CreateAssetHandle<GPipeline>("image_dialation_pipeline", _ds2);
			if (!imageDialationPipeline)
			{
				TRC_ERROR("Failed to initialize or create image_dialation_pipeline");
				return;
			}
		};

		{
			Ref<GShader> VertShader;
			Ref<GShader> FragShader;

			Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

			Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("obj_highlight_blend.frag.glsl", "obj_highlight_blend.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FINAL_BLEND_PASS");

			//Enable_Blending(_ds2);

			DepthStencilState dp_state = {};
			dp_state.depth_test_enable = false;
			dp_state.depth_write_enable = false;
			dp_state.minDepth = 0.0f;
			dp_state.maxDepth = 1.0f;
			dp_state.stencil_test_enable = true;

			StencilState stencil_state = {};
			stencil_state.compareOp = CompareOp::COMPARE_EQUAL;
			stencil_state.compare_mask = 0xff;
			stencil_state.depth_failOp = StencilOp::STENCIL_KEEP;
			stencil_state.failOp = StencilOp::STENCIL_REPLACE;
			stencil_state.passOp = StencilOp::STENCIL_KEEP;
			stencil_state.reference = 2;
			stencil_state.write_mask = 0xff;

			dp_state.stencil_state = stencil_state;

			_ds2.depth_sten_state = dp_state;
			_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };

			finalBlendPipeline = asset_manager->CreateAssetHandle<GPipeline>("final_highlight_blend_pipeline", _ds2);
			if (!finalBlendPipeline)
			{
				TRC_ERROR("Failed to initialize or create final_highlight_blend_pipeline");
				return;
			}
		};


	}

	void ObjectHighlightPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	static std::vector<glm::vec4> clear_values = {
		glm::vec4(0.0f),
		glm::vec4(0.0f),
		glm::vec4(0.0f),
		glm::vec4(0.0f)
	};

	void ObjectHighlightPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index)
	{

		FrameData& frame_data = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();

		uint32_t width = render_graph->GetResource(frame_data.ldr_index).resource_data.texture.width;
		uint32_t height = render_graph->GetResource(frame_data.ldr_index).resource_data.texture.height;

		TextureDesc color_desc = {};
		color_desc.m_addressModeU = color_desc.m_addressModeV = color_desc.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		color_desc.m_attachmentType = AttachmentType::COLOR;
		color_desc.m_flag = BindFlag::RENDER_TARGET_BIT;
		color_desc.m_format = Format::R8G8B8A8_UNORM;
		color_desc.m_width = width;
		color_desc.m_height = height;
		color_desc.m_minFilterMode = color_desc.m_magFilterMode = FilterMode::LINEAR;
		color_desc.m_mipLevels = color_desc.m_numLayers = 1;
		color_desc.m_usage = UsageFlag::DEFAULT;

		auto obj_render_pass = render_graph->AddPass("OBJ_HIGHLIGHT_RENDER_PASS", GPU_QUEUE::GRAPHICS);

		uint32_t color_buffer_index = obj_render_pass->CreateAttachmentOutput("color_buffer", color_desc);
		obj_render_pass->SetDepthStencilInput(gbuffer_data.depth_index);

		obj_render_pass->SetClearValues(clear_values);

		obj_render_pass->SetRunCB([width, height, this](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {

			Viewport view_port = m_renderer->_viewPort;
			Rect2D rect = m_renderer->_rect;
			view_port.width = static_cast<float>(width);
			view_port.height = static_cast<float>(height);

			uint32_t edge_offset = 2;

			rect.top += edge_offset;
			rect.left += edge_offset;
			rect.right = width - (edge_offset * 2);
			rect.bottom = height - (edge_offset * 2);
			RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
			RenderFunc::BindRect(m_renderer->GetDevice(), rect);

			// Render Selected objects
			RenderGraphFrameData& graph_data = *renderer->GetRenderGraphData(render_graph_index);

			Camera* _camera = graph_data._camera;
			glm::mat4 proj = _camera->GetProjectionMatix();
			glm::mat4 view = _camera->GetViewMatrix();
			glm::vec3 view_position = _camera->GetPosition();
			glm::mat4 view_proj = proj * view;

			std::vector<Entity>& selected_entities = *object_highlight[render_graph_index];

			for (Entity& entity : selected_entities)
			{
				HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
				if (ModelComponent* mdl = entity.TryGetComponent<ModelComponent>())
				{
					glm::mat4* M_model = &hi.transform;
					Model* _model = mdl->_model.get();
					Ref<GPipeline> sp = m_pipeline;

					RenderFunc::OnDrawStart(renderer->GetDevice(), sp.get());
					RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
					RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4), 0, render_graph_index);
					RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3), 0, render_graph_index);
					RenderFunc::SetPipelineData(sp.get(), "_model", ShaderResourceStage::RESOURCE_STAGE_DRAW_CALL, M_model, sizeof(glm::mat4), 0, render_graph_index);
					RenderFunc::BindPipeline(renderer->GetDevice(), sp.get());
					RenderFunc::BindPipeline_(sp.get(), render_graph_index);
					RenderFunc::BindVertexBuffer(renderer->GetDevice(), _model->GetVertexBuffer());
					RenderFunc::BindIndexBuffer(renderer->GetDevice(), _model->GetIndexBuffer());

					RenderFunc::DrawIndexed(renderer->GetDevice(), 0, _model->GetIndexCount());
					RenderFunc::OnDrawEnd(renderer->GetDevice(), sp.get());
				}


			}


			});


		auto image_dialation_pass = render_graph->AddPass("IMAGE_DIALATION_PASS", GPU_QUEUE::GRAPHICS);

		image_dialation_pass->AddColorAttachmentInput(color_buffer_index);
		image_dialation_pass->AddColorAttachmentInput(frame_data.ldr_index);
		uint32_t image_dialation_index = image_dialation_pass->CreateAttachmentOutput("IMAGE_DIALATION_RESULT", color_desc);

		image_dialation_pass->SetRunCB([width, height, color_buffer_index, frame_data, this](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);
				Viewport view_port = m_renderer->_viewPort;
				Rect2D rect = m_renderer->_rect;
				view_port.width = static_cast<float>(width);
				view_port.height = static_cast<float>(height);

				rect.right = width;
				rect.bottom = height;
				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), imageDialationPipeline.get());
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					imageDialationPipeline.get(),
					"color_buffer",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(color_buffer_index),
					render_graph_index
				);

				RenderFunc::BindRenderGraphTexture(
					render_graph,
					imageDialationPipeline.get(),
					"scene_color_buffer",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(frame_data.ldr_index),
					render_graph_index
				);

				float matrix_size = 1.0f;
				RenderFunc::SetPipelineData(
					imageDialationPipeline.get(),
					"matrix_size",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					&matrix_size,
					sizeof(float),
					0,
					render_graph_index
				);

				RenderFunc::BindPipeline_(imageDialationPipeline.get(), render_graph_index);
				RenderFunc::BindPipeline(m_renderer->GetDevice(), imageDialationPipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), imageDialationPipeline.get());


			});

		auto final_blend_pass = render_graph->AddPass("FINAL_OBJ_HIGHLIGHT_PASS", GPU_QUEUE::GRAPHICS);

		final_blend_pass->AddColorAttachmentInput(image_dialation_index);

		final_blend_pass->AddColorAttachmentOuput(frame_data.ldr_index);
		final_blend_pass->SetDepthStencilInput(gbuffer_data.depth_index);

		final_blend_pass->SetRunCB([width, height, image_dialation_index, this](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);
				Viewport view_port = m_renderer->_viewPort;
				Rect2D rect = m_renderer->_rect;
				view_port.width = static_cast<float>(width);
				view_port.height = static_cast<float>(height);

				rect.right = width;
				rect.bottom = height;
				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), finalBlendPipeline.get());
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					finalBlendPipeline.get(),
					"image_dialate",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(image_dialation_index),
					render_graph_index
				);

				RenderFunc::BindPipeline_(finalBlendPipeline.get(), render_graph_index);
				RenderFunc::BindPipeline(m_renderer->GetDevice(), finalBlendPipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), finalBlendPipeline.get());


			});



	}

	void ObjectHighlightPass::ShutDown()
	{

		m_pipeline.free();
		imageDialationPipeline.free();
		finalBlendPipeline.free();

		RenderFunc::DestroyRenderPass(&m_renderPass);
		RenderFunc::DestroyRenderPass(&imageDialationPass);
		RenderFunc::DestroyRenderPass(&finalBlendPass);
	}

	void ObjectHighlightPass::SetHighlightedEntities(int32_t render_graph_index, std::vector<Entity>* entites)
	{
		object_highlight[render_graph_index] = entites;
	}

}
