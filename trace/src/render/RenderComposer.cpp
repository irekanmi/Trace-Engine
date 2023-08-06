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

		return result;
	}

	void RenderComposer::Shutdowm()
	{
		lighting_pass.ShutDown();
		gbuffer_pass.ShutDown();
	}

	bool RenderComposer::PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings)
	{
		bool result = true;

		frame_graph.Destroy();

		FrameData& fd = black_board.add<FrameData>();
		fd.ldr_index = frame_graph.AddSwapchainResource("swapchain", &m_renderer->_swapChain);
		fd.frame_width = m_renderer->m_frameWidth;
		fd.frame_height = m_renderer->m_frameHeight;

		frame_graph.SetRenderer(m_renderer);
		gbuffer_pass.Setup(&frame_graph, black_board);
		lighting_pass.Setup(&frame_graph, black_board);
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