#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "render_graph/GBufferPass.h"
#include "render_graph/LightingPass.h"
#include "render_graph/SSAO.h"


namespace trace {

	class Renderer;
	class RenderGraph;
	class RGBlackBoard;


	class TRACE_API RenderComposer
	{

	public:
		RenderComposer(){}
		~RenderComposer(){}

		bool Init(Renderer* renderer);
		void Shutdowm();

		bool PreFrame(RenderGraph& frame_graph,RGBlackBoard& black_board, FrameSettings frame_settings);
		bool PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board);

	private:
		Renderer* m_renderer;
		GBufferPass gbuffer_pass;
		LightingPass lighting_pass;
		SSAO ssao_pass;

	protected:


	};

}