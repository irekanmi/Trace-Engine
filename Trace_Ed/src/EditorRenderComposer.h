#pragma once

#include "trace.h"
#include "render/RenderComposer.h"
#include "render/render_graph/UIPass.h"
#include "render/render_graph/WeightedOITPass.h"
#include "passes/EditorUIPass.h"

namespace trace {

	class EditorRenderComposer : public RenderComposer
	{

	public:

		virtual bool Init(Renderer* renderer) override;
		virtual void Shutdowm() override;

		virtual bool PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index = 0) override;
		virtual bool PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, int32_t render_graph_index = 0) override;

	private:
		GBufferPass gbuffer_pass;
		LightingPass lighting_pass;
		SSAO ssao_pass;
		ToneMapPass toneMap_pass;
		ForwardPass forward_pass;
		BloomPass bloom_pass;
		UIPass ui_pass;
		EditorUIPass editor_ui_pass;
		ShadowPass shadow_pass;
		WeightedOITPass weightedOITPass;

		float x = 800.0f;
		float y = 600.0f;
		FrameSettings current_settings;

	protected:

	};

}
