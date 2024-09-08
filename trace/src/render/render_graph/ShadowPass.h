#pragma once


#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"
#include "resource/Ref.h"

namespace trace {

	class GPipeline;

	class TRACE_API ShadowPass : public RenderPass
	{

	public:
		ShadowPass() {}
		~ShadowPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index) override;
		virtual void ShutDown() override;

	private:
		Ref<GPipeline> m_sun_pipeline;
		Ref<GPipeline> m_spot_pipeline;
		Ref<GPipeline> m_point_pipeline;

	protected:


	};

}
