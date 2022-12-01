#pragma once

#include "core/Enums.h"
#include "core/io/Logging.h"
#include "vulkan/vulkan.h"
#include "EASTL/vector.h"


namespace trace {

#ifdef TRC_DEBUG_BUILD
#define VK_ASSERT(func) TRC_ASSERT(func == VK_SUCCESS, #func);
#else
#define VK_ASSERT(func) func
#endif

	struct SwapchainInfo
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		uint32_t format_count;
		eastl::vector<VkSurfaceFormatKHR> formats;
		uint32_t present_count;
		eastl::vector<VkPresentModeKHR> present_modes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphics_queue = -1;
		uint32_t present_queue = -1;
		uint32_t transfer_queue = -1;
		uint32_t compute_queue = -1;
	};

	struct VkSwapChain
	{
		VkSwapchainKHR m_handle;
		VkSurfaceFormatKHR m_format;
		eastl::vector<VkImage> m_images;
		eastl::vector<VkImageView> m_imageViews;
		uint32_t image_count;
		uint8_t frames_in_flight;
	};

	struct VkHandle
	{
		VkInstance m_instance;
		VkAllocationCallbacks* m_alloc_callback = nullptr;
		VkSurfaceKHR m_surface;
#ifdef TRC_DEBUG_BUILD
		VkDebugUtilsMessengerEXT m_debugutils;
#endif
	};

	struct VkDeviceHandle
	{
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkQueue m_transferQueue;

		QueueFamilyIndices m_queues;
		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;
		SwapchainInfo m_swapchainInfo;
	};



}
