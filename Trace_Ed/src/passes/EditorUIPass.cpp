
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
			m_renderer->GetAvaliableRenderPasses()["UI_PASS"] = &m_renderPass;
		};

	}

	void EditorUIPass::Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs)
	{
	}

	void EditorUIPass::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index)
	{

		FrameData& frame_data = black_board.get<FrameData>();

		auto pass = render_graph->AddPass("EDITOR_UI_PASS", GPU_QUEUE::GRAPHICS);

		//pass->AddColorAttachmentInput(render_graph->GetResource(frame_data.ldr_index).resource_name);
		pass->AddColorAttachmentOuput(render_graph->GetResource(frame_data.swapchain_index).resource_name);

		if (render_graph_index == 0)
		{
			RenderComposer* render_composer = Renderer::get_instance()->GetRenderComposer();
			std::vector<RenderGraphInfo>& graphs = render_composer->GetGraphs();
			
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
	}

}