#pragma once

#include <core/Core.h>
#include <core/Enums.h>

#include <render/render_graph/RenderPass.h>
#include <resource/Ref.h>

namespace trace {

	class GPipeline;
	class Renderer;

	class TRACE_API EditorUIPass : public RenderPass
	{

	public:
		EditorUIPass() {}
		~EditorUIPass() {}

		virtual void Init(Renderer* renderer) override;
		virtual void Setup(RenderGraph* render_graph, RenderPassPacket& pass_inputs) override;
		virtual void Setup(RenderGraph* render_graph, RGBlackBoard& black_board) override;
		virtual void ShutDown() override;

	private:
		Ref<GPipeline> m_pipeline;

	protected:


	};

}

