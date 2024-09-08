#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "RenderPass.h"
#include "resource/Ref.h"
#include "render/GRenderPass.h"

namespace trace {

	class GPipeline;

	class TRACE_API BloomPass : public RenderPass
	{

	public:
		BloomPass() {}
		~BloomPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index) override;
		virtual void ShutDown() override;

	private:
		void prefilter(RenderGraph* render_graph, RGBlackBoard& black_board);
		void downsample(RenderGraph* render_graph, RGBlackBoard& black_board);
		void upsample(RenderGraph* render_graph, RGBlackBoard& black_board);

	private:
		Ref<GPipeline> m_prefilterPipeline;
		Ref<GPipeline> m_downSamplePipeline;
		Ref<GPipeline> m_upSamplePipeline;

		GRenderPass m_downSamplePass;
		GRenderPass m_upSamplePass;

	protected:

	};

}

