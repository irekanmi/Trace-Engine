#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"

namespace trace {

	class TRACE_API SSAO : public RenderPass
	{

	public:
		SSAO(){}
		~SSAO(){}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void ShutDown() override;

	private:
		uint32_t positon_index;
		uint32_t normal_index;
		uint32_t ssao_output;
		GRenderPass ssao_blur;
		GTexture noise_tex;

	private:
		TextureDesc ssao_main_desc;
		TextureDesc ssao_blur_desc;

	protected:

	};

}
