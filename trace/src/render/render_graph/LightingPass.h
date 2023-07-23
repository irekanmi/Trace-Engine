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
		virtual void ShutDown() override;

	private:
		uint32_t color_output_index;
		uint32_t gPosition_index;
		uint32_t gNormal_index;
		uint32_t gColor_index;
		Ref<GPipeline> m_pipeline;

	protected:

	};

}
