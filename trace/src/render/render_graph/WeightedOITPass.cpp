#include "pch.h"

#include "WeightedOITPass.h"
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

	void WeightedOITPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo accum_attach;
			accum_attach.attachmant_index = 0;
			accum_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			accum_attach.initial_format = TextureFormat::UNKNOWN;
			accum_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			accum_attach.is_depth = false;
			accum_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			accum_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;
			
			AttachmentInfo reveal_attach;
			reveal_attach.attachmant_index = 1;
			reveal_attach.attachment_format = Format::R8_UNORM;
			reveal_attach.initial_format = TextureFormat::UNKNOWN;
			reveal_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			reveal_attach.is_depth = false;
			reveal_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			reveal_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 2;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_LOAD;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_DISCARD;


			AttachmentInfo att_infos[] = {
				accum_attach,
				reveal_attach,
				depth_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = 3;
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, 0.0f, 0.0f, 0.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;


			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["WOIT_PASS"] = &m_renderPass;
		};

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		if (AppSettings::is_editor)
		{
			Ref<GShader> VertShader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);
			Ref<GShader> FragShader = asset_manager->CreateAssetHandle<GShader>("weighted_oit.frag.glsl", "weighted_oit.frag.glsl", ShaderStage::PIXEL_SHADER);

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
			_ds2.rasteriser_state = { CullMode::NONE, FillMode::SOLID };


			m_pipeline = asset_manager->CreateAssetHandle<GPipeline>("weightedOIT_pass_pipeline", _ds2);
			if (!m_pipeline)
			{
				TRC_ERROR("Failed to initialize or create weightedOIT_pass_pipeline");
				return;
			}

		}
		else
		{
			UUID id = GetUUIDFromName("weightedOIT_pass_pipeline");
			m_pipeline = asset_manager->Load_Runtime<GPipeline>(id);
		}

	}

	static std::vector<glm::vec4> clear_values = { glm::vec4(0.0f), glm::vec4(1.0f) };

	void WeightedOITPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index)
	{

		FrameData& frame_data = black_board.get<FrameData>();
		GBufferData& g_buffer = black_board.get<GBufferData>();

		RenderGraphPass* pass = render_graph->AddPass("WEIGHTED_OIT_PASS_DRAW", GPU_QUEUE::GRAPHICS);

		uint32_t width = frame_data.frame_width;
		uint32_t height = frame_data.frame_height;

		TextureDesc tex_desc = {};
		tex_desc.m_addressModeU = tex_desc.m_addressModeV = tex_desc.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		tex_desc.m_attachmentType = AttachmentType::COLOR;
		tex_desc.m_flag = BindFlag::RENDER_TARGET_BIT;
		tex_desc.m_format = Format::R16G16B16A16_FLOAT;
		tex_desc.m_width = width;
		tex_desc.m_height = height;
		tex_desc.m_minFilterMode = tex_desc.m_magFilterMode = FilterMode::LINEAR;
		tex_desc.m_mipLevels = tex_desc.m_numLayers = 1;
		tex_desc.m_usage = UsageFlag::DEFAULT;

		uint32_t accum = pass->CreateAttachmentOutput("accum", tex_desc);

		tex_desc.m_format = Format::R8_UNORM;
		uint32_t reveal = pass->CreateAttachmentOutput("reveal", tex_desc);

		pass->SetDepthStencilInput(g_buffer.depth_index);

		
		pass->SetClearValues(clear_values);

		pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
			{
				Viewport view_port = m_renderer->_viewPort;
				Rect2D rect = m_renderer->_rect;
				view_port.width = static_cast<float>(width);
				view_port.height = static_cast<float>(height);

				rect.right = width;
				rect.bottom = height;
				RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
				RenderFunc::BindRect(m_renderer->GetDevice(), rect);

				m_renderer->RenderQuads(render_graph_index);
				m_renderer->RenderTextVerts(render_graph_index);

				});


		RenderGraphPass* composite_pass = render_graph->AddPass("WEIGHTED_OIT_PASS_COMPOSITE", GPU_QUEUE::GRAPHICS);
		composite_pass->AddColorAttachmentOuput(frame_data.hdr_index);
		composite_pass->AddColorAttachmentInput(accum);
		composite_pass->AddColorAttachmentInput(reveal);

		composite_pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs)
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

				RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_pipeline.get());
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"accum",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(accum)
				);
				RenderFunc::BindRenderGraphTexture(
					render_graph,
					m_pipeline.get(),
					"reveal",
					ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
					render_graph->GetResource_ptr(reveal)
				);

				RenderFunc::BindPipeline_(m_pipeline.get());
				RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());
				RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
				RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_pipeline.get());


			});

	}

	void WeightedOITPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}