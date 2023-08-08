#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "RenderPass.h"
#include "resource/Ref.h"

#define MAX_NUM_KERNEL 64

namespace trace {

	class GPipeline;

	class TRACE_API SSAO : public RenderPass
	{

	public:
		SSAO(){}
		~SSAO(){}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board) override;
		virtual void ShutDown() override;

	private:
		uint32_t positon_index;
		uint32_t normal_index;
		uint32_t ssao_output;
		GRenderPass ssao_blur;
		GTexture noise_tex;
		std::array<glm::vec3, MAX_NUM_KERNEL> m_kernel;
		Ref<GPipeline> m_pipeline;
		Ref<GPipeline> m_blurPipe;

	private:
		TextureDesc ssao_main_desc;

	protected:

	};

}
