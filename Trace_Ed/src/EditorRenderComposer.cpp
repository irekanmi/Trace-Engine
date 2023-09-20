
#include "EditorRenderComposer.h"
#include <render/render_graph/FrameData.h>
#include "TraceEditor.h"


namespace trace {

	bool EditorRenderComposer::Init(Renderer* renderer)
	{
		bool result = true;

		if (!renderer) result = false;

		m_renderer = renderer;

		gbuffer_pass.Init(m_renderer);
		lighting_pass.Init(m_renderer);
		ssao_pass.Init(m_renderer);
		toneMap_pass.Init(m_renderer);
		forward_pass.Init(m_renderer);
		bloom_pass.Init(m_renderer);
		ui_pass.Init(m_renderer);
		editor_ui_pass.Init(m_renderer);

		return result;
	}
	void EditorRenderComposer::Shutdowm()
	{
		editor_ui_pass.ShutDown();
		ui_pass.ShutDown();
		bloom_pass.ShutDown();
		forward_pass.ShutDown();
		toneMap_pass.ShutDown();
		ssao_pass.ShutDown();
		lighting_pass.ShutDown();
		gbuffer_pass.ShutDown();
	}
	bool EditorRenderComposer::PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings)
	{
		bool result = true;

		FrameData& fd = black_board.add<FrameData>();
		fd.frame_settings = frame_settings;


		fd.hdr_index = INVALID_ID;
		fd.swapchain_index = frame_graph.AddSwapchainResource("swapchain", m_renderer->GetSwapchain());
		fd.frame_width = TraceEditor::get_instance()->m_viewportSize.x;
		fd.frame_height = TraceEditor::get_instance()->m_viewportSize.y;

		TextureDesc gPos = {};
		gPos.m_addressModeU = gPos.m_addressModeV = gPos.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		gPos.m_attachmentType = AttachmentType::COLOR;
		gPos.m_flag = BindFlag::RENDER_TARGET_BIT;
		gPos.m_format = Format::R8G8B8A8_UNORM;
		gPos.m_width = fd.frame_width;
		gPos.m_height = fd.frame_height;
		gPos.m_minFilterMode = gPos.m_magFilterMode = FilterMode::LINEAR;
		gPos.m_mipLevels = gPos.m_numLayers = 1;
		gPos.m_usage = UsageFlag::DEFAULT;

		fd.ldr_index = frame_graph.AddTextureResource("Ldr_Target", gPos);

		frame_graph.SetRenderer(m_renderer);
		gbuffer_pass.Setup(&frame_graph, black_board);
		if (TRC_HAS_FLAG(frame_settings, RENDER_SSAO))
		{
			ssao_pass.Setup(&frame_graph, black_board);
		}
		lighting_pass.Setup(&frame_graph, black_board);
		forward_pass.Setup(&frame_graph, black_board);
		if (TRC_HAS_FLAG(frame_settings, RENDER_BLOOM))
		{
			bloom_pass.Setup(&frame_graph, black_board);
		}
		toneMap_pass.Setup(&frame_graph, black_board);
		editor_ui_pass.Setup(&frame_graph, black_board);
		frame_graph.SetFinalResourceOutput("swapchain");


		frame_graph.Compile();
		frame_graph.Rebuild();

		return result;
	}
	bool EditorRenderComposer::PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board)
	{
		bool result = true;



		return result;
	}
}


