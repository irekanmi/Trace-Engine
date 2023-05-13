#include <pch.h>

#include "GDevice.h"
#include "Renderutils.h"

namespace trace {

	GDevice::GDevice()
	{

	}

	GDevice::~GDevice()
	{

	}

	bool GDevice::Init()
	{
		return RenderFunc::CreateDevice(this);
	}

	void GDevice::ShutDown()
	{
		RenderFunc::DestroyDevice(this);
	}

	void GDevice::DrawElements(GBuffer* vertex_buffer){}
	void GDevice::DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances){}
	void GDevice::DrawIndexed(GBuffer* index_buffer){}
	void GDevice::DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances){}

	void GDevice::BindViewport(Viewport view_port){}
	void GDevice::BindRect(Rect2D rect){}
	void GDevice::BindPipeline(GPipeline* pipeline){}
	void GDevice::BindVertexBuffer(GBuffer* buffer){}
	void GDevice::BindIndexBuffer(GBuffer* buffer){}
	void GDevice::Draw(uint32_t start_vertex, uint32_t count){}
	void GDevice::DrawIndexed(uint32_t first_index, uint32_t count){}
	void GDevice::BeginRenderPass(GRenderPass* render_pass, GFramebuffer* frame_buffer){}
	void GDevice::NextSubpass(GRenderPass* render_pass){}
	void GDevice::EndRenderPass(GRenderPass* render_pass){}


	bool GDevice::BeginFrame(GSwapchain* swapchain) { return false; }
	void GDevice::EndFrame(){}

}

