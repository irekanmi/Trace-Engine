#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"

namespace trace {

	class TRACE_API LightingPass : public RenderPass
	{

	public:
		LightingPass(){}
		~LightingPass(){}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void ShutDown() override;

	private:
	protected:

	};

}
