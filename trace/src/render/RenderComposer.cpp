#include "pch.h"

#include "RenderComposer.h"
#include "Renderer.h"
#include "render_graph/RGBlackBoard.h"
#include "render_graph/RenderGraph.h"
#include "render_graph/FrameData.h"

namespace trace {



	bool RenderComposer::Init(Renderer* renderer)
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

		return result;
	}

	void RenderComposer::Shutdowm()
	{
		bloom_pass.ShutDown();
		forward_pass.ShutDown();
		toneMap_pass.ShutDown();
		ssao_pass.ShutDown();
		lighting_pass.ShutDown();
		gbuffer_pass.ShutDown();
	}

	bool RenderComposer::PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings)
	{
		bool result = true;

		FrameData& fd = black_board.add<FrameData>();
		fd.frame_settings = frame_settings;

		fd.hdr_index = INVALID_ID;
		fd.ldr_index = frame_graph.AddSwapchainResource("swapchain", m_renderer->GetSwapchain());
		fd.frame_width = m_renderer->GetFrameWidth();
		fd.frame_height = m_renderer->GetFrameHeight();

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
		frame_graph.SetFinalResourceOutput("swapchain");


		frame_graph.Compile();
		frame_graph.Rebuild();

		return result;
	}

	bool RenderComposer::PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board)
	{
		bool result = true;



		return result;
	}

}