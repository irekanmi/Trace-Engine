#include "pch.h"

#include "GBufferPass.h"
#include "render/Renderutils.h"
#include "render/Renderer.h"
#include "FrameData.h"
#include "render/ShaderParser.h"
#include "render/GShader.h"
#include "resource/ResourceSystem.h"

namespace trace {
	void GBufferPass::Init(Renderer* renderer)
	{
		m_renderer = renderer;

		{
			AttachmentInfo position_attach;
			position_attach.attachmant_index = 0;
			position_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			position_attach.initial_format = TextureFormat::UNKNOWN;
			position_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			position_attach.is_depth = false;
			position_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			position_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo normal_attach;
			normal_attach.attachmant_index = 1;
			normal_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			normal_attach.initial_format = TextureFormat::UNKNOWN;
			normal_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			normal_attach.is_depth = false;
			normal_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			normal_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo color_attach;
			color_attach.attachmant_index = 2;
			color_attach.attachment_format = Format::R16G16B16A16_FLOAT;
			color_attach.initial_format = TextureFormat::UNKNOWN;
			color_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			color_attach.is_depth = false;
			color_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


			AttachmentInfo depth_attach;
			depth_attach.attachmant_index = 3;
			depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
			depth_attach.initial_format = TextureFormat::UNKNOWN;
			depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
			depth_attach.is_depth = true;
			depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo att_infos[] = {
				position_attach,
				normal_attach,
				color_attach,
				depth_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = 4;
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;


			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);
			m_renderer->_avaliable_passes["GBUFFER_PASS"] = &m_renderPass;
		};

		{
		TextureDesc depth = {};
		depth.m_addressModeU = depth.m_addressModeV = depth.m_addressModeW = AddressMode::REPEAT;
		depth.m_attachmentType = AttachmentType::DEPTH;
		depth.m_flag = BindFlag::DEPTH_STENCIL_BIT;
		depth.m_format = Format::D32_SFLOAT_S8_SUINT;
		depth.m_width = 800;
		depth.m_height = 600;
		depth.m_minFilterMode = depth.m_magFilterMode = FilterMode::LINEAR;
		depth.m_mipLevels = depth.m_numLayers = 1;
		depth.m_usage = UsageFlag::DEFAULT;

		TextureDesc gPos = {};
		gPos.m_addressModeU = gPos.m_addressModeV = gPos.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		gPos.m_attachmentType = AttachmentType::COLOR;
		gPos.m_flag = BindFlag::RENDER_TARGET_BIT;
		gPos.m_format = Format::R16G16B16A16_FLOAT;
		gPos.m_width = 800;
		gPos.m_height = 600;
		gPos.m_minFilterMode = gPos.m_magFilterMode = FilterMode::LINEAR;
		gPos.m_mipLevels = gPos.m_numLayers = 1;
		gPos.m_usage = UsageFlag::DEFAULT;

		position_desc = gPos;
		depth_desc = depth;
		};

		{
			std::string vert_src;
			std::string frag_src;
			GShader VertShader;
			GShader FragShader;
			
			vert_src = ShaderParser::load_shader_file("../assets/shaders/trace_core.shader.vert.glsl");
			frag_src = ShaderParser::load_shader_file("../assets/shaders/g_buffer.frag.glsl");


			RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
			RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);



			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(&VertShader, s_res);
			ShaderParser::generate_shader_resources(&FragShader, s_res);

			PipelineStateDesc _ds;
			_ds.vertex_shader = &VertShader;
			_ds.pixel_shader = &FragShader;
			_ds.resources = s_res;

			AutoFillPipelineDesc(
				_ds
			);
			_ds.render_pass = Renderer::get_instance()->GetRenderPass("GBUFFER_PASS");
			_ds.blend_state.alpha_to_blend_coverage = false;


			if (!ResourceSystem::get_instance()->CreatePipeline(_ds, "gbuffer_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create default g_buffer pipeline");
				RenderFunc::DestroyShader(&VertShader);
				RenderFunc::DestroyShader(&FragShader);
				return;
			}

			RenderFunc::DestroyShader(&VertShader);
			RenderFunc::DestroyShader(&FragShader);

			vert_src.clear();
			frag_src.clear();

		};

	}
	void GBufferPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
		auto pass = render_graph->AddPass("GBUFFER_PASS", GPU_QUEUE::GRAPHICS);
		position_index = pass_inputs.outputs[0];
		normal_index = pass_inputs.outputs[1];
		color_index = pass_inputs.outputs[2];
		depth_index = pass_inputs.outputs[3];

		pass->CreateAttachmentOutput(
			render_graph->GetResource(position_index).resource_name,
			{}
		);
		pass->CreateAttachmentOutput(
			render_graph->GetResource(normal_index).resource_name,
			{}
		);
		pass->CreateAttachmentOutput(
			render_graph->GetResource(color_index).resource_name,
			{}
		);

		pass->CreateDepthAttachmentOutput(
			render_graph->GetResource(depth_index).resource_name,
			{}
		);

		pass->SetRunCB([&](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);


				for (uint32_t i = 0; i < m_renderer->m_listCount; i++)
				{
					for (Command& cmd : m_renderer->m_cmdList[i]._commands)
					{
						cmd.func(cmd.params);
					}
				}
			});

		pass->SetResizeCB([&](RenderGraph* graph, RenderGraphPass* pass, uint32_t width, uint32_t height)
			{
				TextureDesc desc;
				desc.m_width = width;
				desc.m_height = height;

				graph->ModifyTextureResource(graph->GetResource(position_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(normal_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(color_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(depth_index).resource_name, desc);
			});
	}
	void GBufferPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{
		GBufferData& gbuffer_data = black_board.add<GBufferData>();
		FrameData& fd = black_board.get<FrameData>();
		auto pass = render_graph->AddPass("GBUFFER_PASS", GPU_QUEUE::GRAPHICS);
		position_desc.m_width = fd.frame_width;
		position_desc.m_height = fd.frame_height;
		depth_desc.m_width = fd.frame_width;
		depth_desc.m_height = fd.frame_height;
		position_index = render_graph->AddTextureResource("gPosition", position_desc);
		normal_index = render_graph->AddTextureResource("gNormal", position_desc);
		color_index = render_graph->AddTextureResource("gColor", position_desc);
		depth_index = render_graph->AddTextureResource("depth", depth_desc);
		
		gbuffer_data.position_index = position_index;
		gbuffer_data.normal_index = normal_index;
		gbuffer_data.color_index = color_index;
		gbuffer_data.depth_index = depth_index;

		pass->CreateAttachmentOutput(
			render_graph->GetResource(position_index).resource_name,
			{}
		);
		pass->CreateAttachmentOutput(
			render_graph->GetResource(normal_index).resource_name,
			{}
		);
		pass->CreateAttachmentOutput(
			render_graph->GetResource(color_index).resource_name,
			{}
		);

		pass->CreateDepthAttachmentOutput(
			render_graph->GetResource(depth_index).resource_name,
			{}
		);

		pass->SetRunCB([&](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);


				m_renderer->RenderOpaqueObjects();
			});

		pass->SetResizeCB([&](RenderGraph* graph, RenderGraphPass* pass, uint32_t width, uint32_t height)
			{
				TextureDesc desc;
				desc.m_width = width;
				desc.m_height = height;

				graph->ModifyTextureResource(graph->GetResource(position_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(normal_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(color_index).resource_name, desc);
				graph->ModifyTextureResource(graph->GetResource(depth_index).resource_name, desc);
			});

	}
	void GBufferPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}