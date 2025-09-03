#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"
#include "resource/Ref.h"

namespace trace {

	class GPipeline;

	class WeightedOITPass : public RenderPass
	{

	public:
		WeightedOITPass() {}
		~WeightedOITPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index = 0) override;
		virtual void ShutDown() override;

	private:
		Ref<GPipeline> m_pipeline;
		GRenderPass composite_pass;


	protected:


	};

}
