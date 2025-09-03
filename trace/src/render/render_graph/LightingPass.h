#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"
#include "resource/Ref.h"


namespace trace {
	class GPipeline;

	

	class TRACE_API LightingPass : public RenderPass
	{

	public:
		LightingPass(){}
		~LightingPass(){}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index = 0) override;

		virtual void ShutDown() override;

	private:
		uint32_t color_output_index;
		uint32_t gPosition_index;
		uint32_t gNormal_index;
		uint32_t gColor_index;
		uint32_t frame_count = 0;
		Ref<GPipeline> m_pipeline;

		TextureDesc output_desc;

	protected:

	};

}
