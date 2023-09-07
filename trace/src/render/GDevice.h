#pragma once

#include "core/Core.h"
#include "GBuffer.h"
#include "GPipeline.h"
#include "Graphics.h"
#include "GTexture.h"


namespace trace {

	class GRenderPass;
	class GFramebuffer;
	class GSwapchain;
	struct Viewport;

	class TRACE_API GDevice
	{

	public:
		GDevice();
		virtual ~GDevice();

		bool Init() ;
		void ShutDown() ;
		void DrawElements(GBuffer* vertex_buffer) ;
		void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) ;
		void DrawIndexed(GBuffer* index_buffer) ;
		void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) ;
		void BindViewport(Viewport view_port) ;
		void BindRect(Rect2D rect) ;
		void BindPipeline(GPipeline* pipeline) ;
		void BindVertexBuffer(GBuffer* buffer) ;
		void BindIndexBuffer(GBuffer* buffer) ;
		void Draw(uint32_t start_vertex, uint32_t count) ;
		void DrawIndexed(uint32_t first_index, uint32_t count) ;
		void BeginRenderPass(GRenderPass* render_pass, GFramebuffer* frame_buffer) ;
		void NextSubpass(GRenderPass* render_pass) ;
		void EndRenderPass(GRenderPass* render_pass) ;
		bool BeginFrame(GSwapchain* swapchain) ;
		void EndFrame() ;

		void* GetNativePtr() { return nullptr; }

		GHandle* GetRenderHandle() { return &m_renderhandle; }

		GPipeline* m_pipeline = nullptr;
	private:
		GHandle m_renderhandle;

	protected:

	};

}


