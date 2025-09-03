#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "RenderPass.h"

namespace trace {


	class TRACE_API ForwardPass : public RenderPass
	{

	public:
		ForwardPass() {}
		~ForwardPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index = 0) override;
		virtual void ShutDown() override;

	private:
	protected:

	};


}
