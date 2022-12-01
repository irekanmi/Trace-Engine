#pragma once
#include "vulkan/vulkan.h"
#include "VKtypes.h"
#include "EASTL/vector.h"
#include "core/Window.h"

namespace vk {

	VkResult _CreateInstance(trace::VkHandle* instance);
	void _DestroyInstance(trace::VkHandle* instance);
	bool _FindLayer(const char* layer, eastl::vector<VkLayerProperties>& layers);
	bool _FindExtension(const char* extension, eastl::vector<VkExtensionProperties>& extensions);
	void EnableValidationlayers(trace::VkHandle* instance);
	void DisableValidationlayers(trace::VkHandle* instance);
	VkResult _CreateDevice(trace::VkDeviceHandle* device, trace::VkHandle* instance);
	void _DestoryDevice(trace::VkDeviceHandle* device, trace::VkHandle* instance);
	VkResult _CreateSurface(trace::VkHandle* instance);
	void _DestorySurface(trace::VkHandle* instance);
	VkPhysicalDevice GetPhysicalDevice(trace::VkHandle* instance);
	void RatePhysicalDevice(VkPhysicalDevice phy_device, uint32_t& score);
	VkResult _CreateSwapchain(trace::VkHandle* instance, trace::VkDeviceHandle* device, trace::VkSwapChain* swapchain, uint32_t width, uint32_t height);
	void _DestroySwapchain(trace::VkHandle* instance, trace::VkDeviceHandle* device, trace::VkSwapChain* swapchain);
	VkResult _RecreateSwapchain(trace::VkHandle* instance, trace::VkDeviceHandle* device, trace::VkSwapChain* swapchain, uint32_t width, uint32_t height);
}
