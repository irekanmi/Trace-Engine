#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "RenderPass.h"
#include "resource/Ref.h"

namespace trace {

	class GPipeline;

	class TRACE_API CustomPass : public RenderPass
	{

	public:
		CustomPass() {}
		~CustomPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void ShutDown() override;

	private:
		uint32_t color_input_index;
		uint32_t color_output_index;
		Ref<GPipeline> m_pipeline;
	protected:

	};


}
