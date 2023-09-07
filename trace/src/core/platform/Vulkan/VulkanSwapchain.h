#pragma once

#include "render/GSwapchain.h"
#include "VKtypes.h"
#include "VulkanTexture.h"


namespace vk {

	bool __CreateSwapchain(trace::GSwapchain* swapchain, trace::GDevice* device, trace::GContext* context);
	bool __DestroySwapchain(trace::GSwapchain* swapchain);
	bool __ResizeSwapchain(trace::GSwapchain* swapchain, uint32_t width, uint32_t height);
	bool __PresentSwapchain(trace::GSwapchain* swapchain);
	bool __GetSwapchainColorBuffer(trace::GSwapchain* swapchain, trace::GTexture* out_texture);
	bool __GetSwapchainDepthBuffer(trace::GSwapchain* swapchain, trace::GTexture* out_texture);
	bool _recreate_swapchain(trace::GSwapchain* swapchain);
}
