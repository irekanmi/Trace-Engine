#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/GRenderPass.h"
#include "RGBlackBoard.h"

namespace trace {

	class Renderer;
	class RenderGraph;
	class GRenderPass;

	struct RenderPassPacket
	{
		uint32_t outputs[8] = {INVALID_ID};
		uint32_t inputs[8] = {INVALID_ID};
	};

	class TRACE_API RenderPass
	{

	public:
		RenderPass() {}
		~RenderPass() {}

		virtual void Init(Renderer* renderer){}
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs){}
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index = 0){}
		virtual void ShutDown(){}

	private:
	protected:
		Renderer* m_renderer = nullptr;
		GRenderPass m_renderPass;

	};


}
