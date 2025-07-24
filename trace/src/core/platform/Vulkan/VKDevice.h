#pragma once

#include "core/Enums.h"
#include "render/Graphics.h"
#include "render/GDevice.h"
#include "VKtypes.h"
#include "core/events/Events.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"





namespace vk {

	bool __CreateDevice(trace::GDevice* device);
	bool __DestroyDevice(trace::GDevice* device);
	bool __BindViewport(trace::GDevice* device, trace::Viewport view_port);
	bool __BindRect(trace::GDevice* device, trace::Rect2D rect);
	bool __BindLineWidth(trace::GDevice* device, float value);
	bool __BindPipeline(trace::GDevice* device, trace::GPipeline* pipeline);
	bool __BindVertexBuffer(trace::GDevice* device, trace::GBuffer* buffer);
	bool __BindVertexBufferBatch(trace::GDevice* device, trace::GBuffer* buffer);
	bool __BindIndexBuffer(trace::GDevice* device, trace::GBuffer* buffer);
	bool __BindIndexBufferBatch(trace::GDevice* device, trace::GBuffer* buffer);
	bool __Draw(trace::GDevice* device, uint32_t start_vertex, uint32_t count);
	bool __DrawInstanced(trace::GDevice* device, uint32_t start_vertex, uint32_t count, uint32_t num_instances);
	bool __DrawIndexed(trace::GDevice* device, uint32_t first_index, uint32_t count);
	bool __DrawIndexedInstanced(trace::GDevice* device, uint32_t first_index, uint32_t count, uint32_t num_instances);
	bool __BeginRenderPass(trace::GDevice* device, trace::GRenderPass* render_pass, trace::GFramebuffer* frame_buffer);
	bool __NextSubpass(trace::GDevice* device, trace::GRenderPass* render_pass);
	bool __EndRenderPass(trace::GDevice* device, trace::GRenderPass* render_pass);
	bool __BeginFrame(trace::GDevice* device, trace::GSwapchain* swapchain);
	bool __EndFrame(trace::GDevice* device);
	bool __OnDrawStart(trace::GDevice* device, trace::GPipeline* pipeline);
	bool __OnDrawEnd(trace::GDevice* device, trace::GPipeline* pipeline);

}
