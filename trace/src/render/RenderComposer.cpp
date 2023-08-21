#include "pch.h"

#include "RenderComposer.h"
#include "Renderer.h"
#include "render_graph/RGBlackBoard.h"
#include "render_graph/RenderGraph.h"
#include "render_graph/FrameData.h"

namespace trace {



	bool RenderComposer::Init(Renderer* renderer)
	{
		bool result = true;

		if (!renderer) result = false;

		m_renderer = renderer;

		gbuffer_pass.Init(m_renderer);
		lighting_pass.Init(m_renderer);
		ssao_pass.Init(m_renderer);
		toneMap_pass.Init(m_renderer);

		return result;
	}

	void RenderComposer::Shutdowm()
	{
		toneMap_pass.ShutDown();
		ssao_pass.ShutDown();
		lighting_pass.ShutDown();
		gbuffer_pass.ShutDown();
	}

	bool RenderComposer::PreFrame(RenderGraph& frame_graph, RGBlackBoard& black_board, FrameSettings frame_settings)
	{
		bool result = true;

		frame_graph.Destroy();

		FrameData& fd = black_board.add<FrameData>();
		TextureDesc hdr_tex;
		hdr_tex.m_addressModeU = hdr_tex.m_addressModeV = hdr_tex.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		hdr_tex.m_attachmentType = AttachmentType::COLOR;
		hdr_tex.m_flag = BindFlag::RENDER_TARGET_BIT;
		hdr_tex.m_format = Format::R16G16B16A16_FLOAT;
		hdr_tex.m_width = m_renderer->m_frameWidth;
		hdr_tex.m_height = m_renderer->m_frameHeight;
		hdr_tex.m_minFilterMode = hdr_tex.m_magFilterMode = FilterMode::LINEAR;
		hdr_tex.m_mipLevels = hdr_tex.m_numLayers = 1;
		hdr_tex.m_usage = UsageFlag::DEFAULT;

		fd.hdr_index = frame_graph.AddTextureResource("Hdr_Target", hdr_tex);
		fd.ldr_index = frame_graph.AddSwapchainResource("swapchain", &m_renderer->_swapChain);
		fd.frame_width = m_renderer->m_frameWidth;
		fd.frame_height = m_renderer->m_frameHeight;

		frame_graph.SetRenderer(m_renderer);
		gbuffer_pass.Setup(&frame_graph, black_board);
		if (TRC_HAS_FLAG(frame_settings, RENDER_SSAO))
		{
			ssao_pass.Setup(&frame_graph, black_board);
		}
		lighting_pass.Setup(&frame_graph, black_board);
		toneMap_pass.Setup(&frame_graph, black_board);
		frame_graph.SetFinalResourceOutput("swapchain");


		frame_graph.Compile();
		frame_graph.Rebuild();

		return result;
	}

	bool RenderComposer::PostFrame(RenderGraph& frame_graph, RGBlackBoard& black_board)
	{
		bool result = true;



		return result;
	}

}