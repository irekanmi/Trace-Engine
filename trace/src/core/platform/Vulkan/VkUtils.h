#pragma once
#include "vulkan/vulkan.h"
#include "VKtypes.h"
#include "EASTL/vector.h"
#include "core/Window.h"

namespace vk {

	// Vulkan Instance
	VkResult _CreateInstance(trace::VKHandle* instance);
	void _DestroyInstance(trace::VKHandle* instance);

	bool _FindLayer(const char* layer, eastl::vector<VkLayerProperties>& layers);
	bool _FindExtension(const char* extension, eastl::vector<VkExtensionProperties>& extensions);

	// Validation layers
	void EnableValidationlayers(trace::VKHandle* instance);
	void DisableValidationlayers(trace::VKHandle* instance);

	// Logical device
	VkResult _CreateDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance);
	void _DestoryDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance);

	// Surfaces
	VkResult _CreateSurface(trace::VKHandle* instance);
	void _DestorySurface(trace::VKHandle* instance);

	VkPhysicalDevice GetPhysicalDevice(trace::VKHandle* instance);
	void RatePhysicalDevice(VkPhysicalDevice phy_device, uint32_t& score);

	// Swapchain
	VkResult _CreateSwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, uint32_t width, uint32_t height);
	void _DestroySwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain);
	VkResult _RecreateSwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, uint32_t width, uint32_t height);
	bool _AcquireSwapchainImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, VkSemaphore image_acquired_sem, trace::VKFence* fence, uint32_t* image_index, uint32_t timeout_ns);
	void _PresentSwapchainImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, VkQueue graphics_queue, VkQueue present_queue, VkSemaphore render_complete, uint32_t* present_image_index);

	// Images
	VkResult _CreateImage(trace::VKHandle* instance,trace::VKDeviceHandle* device, trace::VKImage* out_image, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_prop , VkImageAspectFlags aspect_flags, uint32_t width, uint32_t height);
	VkResult _CreateImageView(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkImageView* out_image_view, VkFormat format, VkImageAspectFlags aspect_flags, VkImage* image);
	void _DestroyImageView(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkImageView* image_view);
	void _DestroyImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* image);


	uint32_t FindMemoryIndex(trace::VKDeviceHandle* device, uint32_t memory_type, VkMemoryPropertyFlags mem_prop);

	// Command Buffer
	void _AllocateCommandBuffer(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* out_command_buffer, bool is_primary = true);
	void _FreeCommandBuffer(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* command_buffer);

	void _BeginCommandBuffer(trace::VKCommmandBuffer* command_buffer, trace::CommandBufferUsage usage);
	void _BeginCommandBufferSingleUse(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* command_buffer);
	void _EndCommandBuffer(trace::VKCommmandBuffer* command_buffer);
	void _EndCommandBufferSingleUse(trace::VKDeviceHandle* device, VkCommandPool command_pool, VkQueue queue, trace::VKCommmandBuffer* command_buffer);
	void _CommandBufferSubmitted(trace::VKCommmandBuffer* command_buffer);
	void _CommandBuffer_Reset(trace::VKCommmandBuffer* command_buffer);

	void _CreateCommandBuffers(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkCommandPool command_pool, eastl::vector<trace::VKCommmandBuffer> &buffers);

	// FrameBuffers
	VkResult _CreateFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer, eastl::vector<VkImageView> attachments, trace::VKRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t attachment_count);
	void _DestoryFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer);
	void _RegenerateFrameBuffers(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, trace::VKRenderPass* render_pass);

	// Fences
	void _CreateFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence, bool signaled);
	bool _WaitFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence, uint64_t timeout_ns);
	void _DestroyFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence);
	void _FenceReset(trace::VKDeviceHandle* device, trace::VKFence* fence);
	

	// RenderPass
	// There is tendency that the render pass create and dectroy function would change
	VkResult _CreateRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass, glm::vec4 clear_color, glm::vec4 render_area, float depth_value, uint32_t stencil_value, trace::VKSwapChain* swapchain);
	void _DestroyRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass);
	void _BeginRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass, trace::VKCommmandBuffer* command_buffer, VkFramebuffer framebuffer);
	void _EndRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer);

	// Utils
	bool _ResultIsSuccess(VkResult result);
	const char* _GetResultString(VkResult result);
}
