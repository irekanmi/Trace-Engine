#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "render_graph/GBufferPass.h"
#include "render_graph/LightingPass.h"
#include "render_graph/SSAO.h"
#include "render_graph/ToneMapPass.h"
#include "render_graph/ForwardPass.h"
#include "render_graph/BloomPass.h"
#include "render_graph/UIPass.h"
#include "render_graph/ShadowPass.h"
#include "render_graph/RenderGraph.h"

#include <functional>
#include <unordered_map>


namespace trace {

	class Renderer;
	class RenderGraph;
	class RGBlackBoard;

	struct RenderGraphController
	{
		std::function<bool()> should_render;
		std::function<void(RenderGraph&, RGBlackBoard&, FrameSettings, int32_t)> build_graph;//TODO: To be used later
		int32_t graph_index = -1;
	};

	struct RenderGraphInfo
	{
		RenderGraph graph;
		bool built = false;
	};

	class RenderComposer
	{

	public:
		RenderComposer(){}
		~RenderComposer(){}

		virtual bool Init(Renderer* renderer);
		virtual void Shutdowm();

		virtual bool PreFrame(RenderGraph& frame_graph,RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index = 0);
		virtual bool PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, int32_t render_graph_index = 0);

		virtual void Render(float deltaTime, FrameSettings frame_settings);
		virtual void DestroyGraphs();
		virtual void SetGraphsCount(uint32_t graph_count);
		virtual RenderGraph* GetRenderGraph(uint32_t graph_index);
		virtual std::vector<RenderGraphInfo>& GetGraphs() { return m_graphs; }

		virtual int32_t BindRenderGraphController(RenderGraphController controller, const std::string& controller_name);
		virtual bool UnBindRenderGraphController(const std::string& controller_name);

	private:
		GBufferPass gbuffer_pass;
		LightingPass lighting_pass;
		SSAO ssao_pass;
		ToneMapPass toneMap_pass;
		ForwardPass forward_pass;
		BloomPass bloom_pass;
		UIPass ui_pass;
		ShadowPass shadow_pass;


	protected:
		Renderer* m_renderer;
		std::vector<RenderGraphInfo> m_graphs;//Used for rendering
		std::vector<RenderGraphController> m_controllers;
		std::unordered_map<std::string, int32_t> m_controllersMap;

	};

}
