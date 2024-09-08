#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "render_graph/GBufferPass.h"
#include "render_graph/LightingPass.h"
#include "render_graph/SSAO.h"
#include "render_graph/ToneMapPass.h"
#include "render_graph/ForwardPass.h"
#include "render_graph/BloomPass.h"
#include "render_graph/UIPass.h"
#include "render_graph/ShadowPass.h"


namespace trace {

	class Renderer;
	class RenderGraph;
	class RGBlackBoard;


	class TRACE_API RenderComposer
	{

	public:
		RenderComposer(){}
		~RenderComposer(){}

		virtual bool Init(Renderer* renderer);
		virtual void Shutdowm();

		virtual bool PreFrame(RenderGraph& frame_graph,RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index = 0);
		virtual bool PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, int32_t render_graph_index = 0);

	private:
		GBufferPass gbuffer_pass;
		LightingPass lighting_pass;
		SSAO ssao_pass;
		ToneMapPass toneMap_pass;
		ForwardPass forward_pass;
		BloomPass bloom_pass;
		UIPass ui_pass;

	protected:
		Renderer* m_renderer;

	};

}
