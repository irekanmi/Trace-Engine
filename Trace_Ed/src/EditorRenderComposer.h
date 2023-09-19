#pragma once

#include "trace.h"
#include <render/RenderComposer.h>
#include <render/render_graph/UIPass.h>
#include "passes/EditorUIPass.h"

namespace trace {

	class EditorRenderComposer : public RenderComposer
	{

	public:

		virtual bool Init(Renderer* renderer) override;
		virtual void Shutdowm() override;

		virtual bool PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings) override;
		virtual bool PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board) override;

	private:
		GBufferPass gbuffer_pass;
		LightingPass lighting_pass;
		SSAO ssao_pass;
		ToneMapPass toneMap_pass;
		ForwardPass forward_pass;
		BloomPass bloom_pass;
		UIPass ui_pass;
		EditorUIPass editor_ui_pass;

	protected:

	};

}
