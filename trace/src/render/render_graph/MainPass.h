#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"

namespace trace {

	class TRACE_API MainPass : public RenderPass
	{

	public:

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void ShutDown() override;

	private:
		uint32_t color_output_index;
		uint32_t depth_index;
	protected:

	};

}
