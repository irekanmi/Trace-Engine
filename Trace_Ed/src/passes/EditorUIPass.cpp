
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
			m_renderer->_avaliable_passes["UI_PASS"] = &m_renderPass;
		};

	}

	void EditorUIPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void EditorUIPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{

		FrameData& frame_data = black_board.get<FrameData>();

		auto pass = render_graph->AddPass("EDITOR_UI_PASS", GPU_QUEUE::GRAPHICS);

		pass->AddColorAttachmentInput(render_graph->GetResource(frame_data.ldr_index).resource_name);
		pass->AddColorAttachmentOuput(render_graph->GetResource(frame_data.swapchain_index).resource_name);



		pass->SetRunCB([=](std::vector<uint32_t>& inputs)
			{
				RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
				RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);

				void* tex_render = nullptr;
				UIFunc::GetDrawRenderGraphTextureHandle(render_graph->GetResource_ptr(frame_data.ldr_index), tex_render);
				TraceEditor::get_instance()->RenderViewport(tex_render);

				UIFunc::UIRenderFrame(m_renderer);

			});

	}

	void EditorUIPass::ShutDown()
	{
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}

}