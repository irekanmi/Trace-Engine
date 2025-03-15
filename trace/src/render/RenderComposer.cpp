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
		ui_pass.Init(m_renderer);
		shadow_pass.Init(m_renderer);

		m_graphs.resize(1);
		m_graphsBlackBoard.resize(1);


		return result;
	}

	void RenderComposer::Shutdowm()
	{

		for (uint32_t i = 0; i < m_graphs.size(); i++)
		{
			m_graphs[i].Destroy();
		}

		shadow_pass.ShutDown();
		ui_pass.ShutDown();
		bloom_pass.ShutDown();
		forward_pass.ShutDown();
		toneMap_pass.ShutDown();
		ssao_pass.ShutDown();
		lighting_pass.ShutDown();
		gbuffer_pass.ShutDown();
	}

	bool RenderComposer::PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index)
	{
		bool result = true;

		FrameData& fd = black_board.add<FrameData>();
		fd.frame_settings = frame_settings;

		fd.hdr_index = INVALID_ID;
		fd.ldr_index = frame_graph.AddSwapchainResource("swapchain", m_renderer->GetSwapchain());
		fd.frame_width = m_renderer->GetFrameWidth();
		fd.frame_height = m_renderer->GetFrameHeight();

		frame_graph.SetRenderer(m_renderer);

		shadow_pass.Setup(&frame_graph, black_board, render_graph_index);
		gbuffer_pass.Setup(&frame_graph, black_board, render_graph_index);
		if (TRC_HAS_FLAG(frame_settings, RENDER_SSAO))
		{
			ssao_pass.Setup(&frame_graph, black_board, render_graph_index);
		}
		lighting_pass.Setup(&frame_graph, black_board, render_graph_index);
		forward_pass.Setup(&frame_graph, black_board, render_graph_index);
		if (TRC_HAS_FLAG(frame_settings, RENDER_BLOOM))
		{
			bloom_pass.Setup(&frame_graph, black_board, render_graph_index);
		}
		toneMap_pass.Setup(&frame_graph, black_board, render_graph_index);
		frame_graph.SetFinalResourceOutput("swapchain");


		frame_graph.Compile();
		frame_graph.Rebuild();

		return result;
	}

	bool RenderComposer::PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, int32_t render_graph_index)
	{
		bool result = true;



		return result;
	}

	void RenderComposer::Render(float deltaTime, FrameSettings frame_settings)
	{
		for (uint32_t i = 0; i < m_graphs.size(); i++)
		{
			RGBlackBoard black_board;
			PreFrame(m_graphs[i], black_board, frame_settings, i);

			m_graphs[i].Execute(i);
			
		}
	}

	bool RenderComposer::ComposeGraph(FrameSettings frame_settings)
	{
		for (uint32_t i = 0; i < m_graphs.size(); i++)
		{
			PreFrame(m_graphs[i], m_graphsBlackBoard[i], frame_settings, i);
		}

		return true;
	}

	bool RenderComposer::ReComposeGraph(FrameSettings frame_settings)
	{
		for (uint32_t i = 0; i < m_graphs.size(); i++)
		{
			m_graphs[i].Destroy();
		}
		return ComposeGraph(frame_settings);
	}

	void RenderComposer::DestroyGraphs()
	{
		for (uint32_t i = 0; i < m_graphs.size(); i++)
		{
			m_graphs[i].Destroy();
		}
	}

	bool RenderComposer::recompose_graph(uint32_t index, FrameSettings frame_settings)
	{
		m_graphs[index].Destroy();
		return compose_graph(index, frame_settings);
	}

	bool RenderComposer::compose_graph(uint32_t index, FrameSettings frame_settings)
	{
		PreFrame(m_graphs[index], m_graphsBlackBoard[index], frame_settings, index);
		return true;
	}

}