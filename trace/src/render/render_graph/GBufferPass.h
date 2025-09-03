#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"

namespace trace {



	class TRACE_API GBufferPass : public RenderPass
	{

	public:
		GBufferPass(){}
		~GBufferPass(){}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index = 0) override;
		virtual void ShutDown() override;

	private:
		uint32_t position_index;
		uint32_t normal_index;
		uint32_t color_index;
		uint32_t depth_index;

		TextureDesc position_desc;
		TextureDesc depth_desc;

	protected:


	};

}
