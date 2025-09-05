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


		SetGraphsCount(MAX_RENDER_GRAPH);


		return result;
	}

	void RenderComposer::Shutdowm()
	{

		DestroyGraphs();

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
		bool result = false;


		RenderGraphController& controller = m_controllers[render_graph_index];
		bool can_build_graph = controller.should_render && controller.build_graph && controller.graph_index >= 0;
		if (can_build_graph)
		{
			result = controller.should_render();
		}
		m_graphs[render_graph_index].built = result;
		if (!result)
		{
			return result;
		}
		
		controller.build_graph(frame_graph, black_board, frame_settings, render_graph_index);

		frame_graph.Compile();
		frame_graph.Rebuild(render_graph_index);

		return result;
	}

	bool RenderComposer::PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, int32_t render_graph_index)
	{
		bool result = true;



		return result;
	}

	void RenderComposer::Render(float deltaTime, FrameSettings frame_settings)
	{
		int32_t last_index = m_graphs.size() - 1;
		for (int32_t i = last_index; i >= 0; i--)
		{
			RGBlackBoard black_board;
			PreFrame(m_graphs[i].graph, black_board, frame_settings, i);

			if (!m_graphs[i].built)
			{
				continue;
			}
			m_graphs[i].graph.Execute(i);
			
		}
	}

	void RenderComposer::DestroyGraphs()
	{
		int32_t last_index = m_graphs.size() - 1;
		for (int32_t i = last_index; i >= 0; i--)
		{
			m_graphs[i].graph.Destroy();
		}
	}

	void RenderComposer::SetGraphsCount(uint32_t graph_count)
	{
		m_graphs.resize(graph_count);
		m_controllers.resize(graph_count);
		for (RenderGraphInfo& i : m_graphs)
		{
			i.graph.SetRenderer(m_renderer);
		}
	}

	RenderGraph* RenderComposer::GetRenderGraph(uint32_t graph_index)
	{
		int32_t last_index = m_graphs.size() - 1;
		if (graph_index <= last_index)
		{
			return &m_graphs[graph_index].graph;
		}
		return nullptr;
	}

	int32_t RenderComposer::BindRenderGraphController(RenderGraphController controller, const std::string& controller_name)
	{
		auto it = m_controllersMap.find(controller_name);
		if (it != m_controllersMap.end())
		{
			TRC_WARN("Controller: {} has already been added, Function: {}", controller_name, __FUNCTION__);
			return -1;
		}

		for (int32_t i = 0; i < m_controllers.size(); i++)
		{
			if (m_controllers[i].graph_index < 0)
			{
				m_controllers[i] = controller;
				m_controllers[i].graph_index = i;
				m_controllersMap[controller_name] = i;
				return i;
			}
		}

		return -1;
	}
	bool RenderComposer::UnBindRenderGraphController(const std::string& controller_name)
	{
		auto it = m_controllersMap.find(controller_name);
		if (it == m_controllersMap.end())
		{
			TRC_WARN("Controller: {} is not part of available controllers, Function: {}", controller_name, __FUNCTION__);
			return false;
		}

		m_controllers[it->second].graph_index = -1;
		m_controllersMap.erase(controller_name);

		return true;
	}
}