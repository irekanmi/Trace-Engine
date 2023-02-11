#include "pch.h"

#include "VkUtils.h"
#include "EASTL/vector.h"
#include "core/Platform.h"
#include "core/Application.h"
#include "vulkan/vulkan_win32.h"
#include "EASTL/map.h"
#include "VulkanShader.h"
#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_glsl.hpp"

struct PhyScr
{
	VkPhysicalDevice phy_device;
	uint32_t score;
};

/* 

		createInstance();	|_/
		setupDebugMessenger();	|_/
		createSurface();	|_/
		pickPhysicalDevice();	|_/
		createLogicalDevice();	|_/
		createSwapChain();	|_/
		createImageViews(); |_/
		createRenderpass(); |_/
		createDescriptorSetLayout();
		createGraphicsPipeline(); |_/
		createDepthResources();	|_/
		createFramebuffers(); |_/
		createCommandPool(); |_/
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createVertexBuffers(); |_/
		createIndexBuffer(); |_/
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSet();
		createCommandBuffer(); |_/
		createSyncObjects(); |_/

*/

static bool FindDepthFormat(trace::VKDeviceHandle* device)
{
	eastl::vector<VkFormat> formats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM
	};

	uint32_t flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	for (auto& i : formats)
	{
		VkFormatProperties fmt_prop;
		vkGetPhysicalDeviceFormatProperties(device->m_physicalDevice, i, &fmt_prop);

		if ((fmt_prop.linearTilingFeatures & flags) == flags)
		{
			device->m_depthFormat = i;
			return true;
		}
		else if ((fmt_prop.optimalTilingFeatures & flags) == flags)
		{
			device->m_depthFormat = i;
			return true;
		}

	}

	TRC_ERROR("Unable to find depth format");
	return false;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {



	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	{
		TRC_ERROR(pCallbackData->pMessage);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
	{
		TRC_INFO(pCallbackData->pMessage);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	{
		TRC_CRITICAL(pCallbackData->pMessage);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	{
		TRC_WARN(pCallbackData->pMessage);
		break;
	}
	}

	return VK_FALSE;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance,
			"vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator,
			pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance,
			"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;

namespace vk {

	eastl::vector<const char*> g_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	eastl::vector<const char*> g_extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
	};



	VkResult _CreateInstance(trace::VKHandle* instance)
	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.apiVersion = VK_API_VERSION_1_2;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pApplicationName = _NAME_;
		app_info.pEngineName = _ENGINE_NAME_;
		app_info.pNext = nullptr;


		uint32_t ext_count = 0;
		eastl::vector<const char*> plat_ext;
		trace::Platform::GetExtensions(ext_count, plat_ext);

		eastl::vector<const char*> extensions;

		for (uint32_t i = 0; i < ext_count; i++)
		{
			extensions.push_back(plat_ext[i]);
		}

#ifdef TRC_DEBUG_BUILD
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		uint32_t layer_count = 0;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		eastl::vector<VkLayerProperties> layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());


		for (auto& i : g_validation_layers)
		{
			bool layer_found = false;

			layer_found = _FindLayer(i, layers);

			if (!layer_found)
			{
				TRC_ERROR("Vaildation layer not found %s", i);
				TRC_ASSERT(false, "Vaildation layer not found %s", i);
				break;
			}
		}

		ext_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
		eastl::vector<VkExtensionProperties> exts(ext_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, exts.data());

		for (auto& i : g_extensions)
		{
			bool ext_found = false;

			ext_found = _FindExtension(i, exts);

			if (ext_found)
			{
				extensions.push_back(i);
			}
			else
			{
				TRC_ERROR("Extension not found %s", i);
				TRC_ASSERT(false, "Extension not found %s", i);
				break;
			}

		}

		VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
#ifdef TRC_DEBUG_BUILD
		create_info.enabledLayerCount = g_validation_layers.size();
		create_info.ppEnabledLayerNames = g_validation_layers.data();
#endif
		create_info.enabledExtensionCount = extensions.size();
		create_info.ppEnabledExtensionNames = extensions.data();
		create_info.pNext = nullptr;

		return vkCreateInstance(&create_info, instance->m_alloc_callback, &instance->m_instance);
	}

	void _DestroyInstance(trace::VKHandle* instance)
	{
		vkDestroyInstance(instance->m_instance, instance->m_alloc_callback);
	}

	bool _FindLayer(const char* layer, eastl::vector<VkLayerProperties>& layers)
	{
		bool found = 0;

		for (auto& i : layers)
		{
			if (strcmp(layer, i.layerName) == 0)
			{
				found = true;
				break;
			}
		}

		return found;
	}

	bool _FindExtension(const char* extension, eastl::vector<VkExtensionProperties>& extensions)
	{
		bool found = 0;

		for (auto& i : extensions)
		{
			if (strcmp(extension, i.extensionName) == 0)
			{
				found = true;
				break;
			}
		}

		return found;
	}

	void EnableValidationlayers(trace::VKHandle* instance)
	{

		uint32_t layer_count = 0;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		eastl::vector<VkLayerProperties> layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());


		for (auto& i : g_validation_layers)
		{
			bool layer_found = false;

			layer_found = _FindLayer(i, layers);
			
			if (!layer_found)
			{
				TRC_ERROR("Vaildation layer not found %s", i);
				break;
			}
		}

		VkDebugUtilsMessengerCreateInfoEXT create_info = {};
		create_info.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity =
			/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debugCallback;
		create_info.pUserData = nullptr; // Optional

#ifdef TRC_DEBUG_BUILD
		VkResult res = CreateDebugUtilsMessengerEXT(instance->m_instance, &create_info, instance->m_alloc_callback, &instance->m_debugutils);

		VK_ASSERT(res);
#endif

	}

	void DisableValidationlayers(trace::VKHandle* instance)
	{
#ifdef TRC_DEBUG_BUILD
		DestroyDebugUtilsMessengerEXT(instance->m_instance, instance->m_debugutils, instance->m_alloc_callback);
#endif
	}

	VkResult _CreateDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance)
	{
		device->frames_in_flight = 2;
		device->m_physicalDevice = GetPhysicalDevice(instance);


		VkPhysicalDeviceProperties phy_prop;
		VkPhysicalDeviceFeatures phy_feat;
		vkGetPhysicalDeviceProperties(device->m_physicalDevice, &phy_prop);
		vkGetPhysicalDeviceFeatures(device->m_physicalDevice, &phy_feat);

		device->m_properties = phy_prop;
		device->m_features = phy_feat;

		uint32_t queue_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device->m_physicalDevice, &queue_count, nullptr);
		eastl::vector<VkQueueFamilyProperties> queue_props(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device->m_physicalDevice, &queue_count, queue_props.data());


		TRC_INFO(" Graphics | Compute | Transfer | Present ")
			int n = 0;
		for (auto& i : queue_props)
		{

			if (i.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				device->m_queues.graphics_queue = n;
			}

			if (i.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				device->m_queues.transfer_queue = n;
			}

			if (i.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				device->m_queues.compute_queue = n;
			}

			VkBool32 present_avaliable = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(device->m_physicalDevice, n, instance->m_surface, &present_avaliable);
			if (present_avaliable)
			{
				device->m_queues.present_queue = n;
			}

			n++;
		}
		

		TRC_TRACE(" %d | %d | %d | %d ", device->m_queues.graphics_queue, device->m_queues.compute_queue, device->m_queues.transfer_queue, device->m_queues.present_queue);


		bool present_share_graphics = device->m_queues.present_queue == device->m_queues.graphics_queue;
		bool transfer_share_graphics = device->m_queues.graphics_queue == device->m_queues.transfer_queue;
		bool present_share_transfer = device->m_queues.present_queue == device->m_queues.present_queue;

		uint32_t index_count = 1;
		if (!present_share_graphics)
		{
			index_count++;
		}
		if (!transfer_share_graphics)
		{
			index_count++;
		}

		if (present_share_transfer)
		{
			index_count--;
		}

		eastl::vector<uint32_t> indices(index_count);
		int8_t index = 0;
		indices[index++] = device->m_queues.graphics_queue;

		if (present_share_transfer)
		{
			indices[index++] = device->m_queues.transfer_queue;
		}
		else
		{
			if (!present_share_graphics)
			{
				indices[index++] = device->m_queues.present_queue;
			}
			if (!transfer_share_graphics)
			{
				indices[index++] = device->m_queues.transfer_queue;
			}

		}

		uint32_t present_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->m_physicalDevice, instance->m_surface, &present_count, nullptr);
		eastl::vector<VkPresentModeKHR> present_modes(present_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->m_physicalDevice, instance->m_surface, &present_count, present_modes.data());

		device->m_swapchainInfo.present_count = present_count;
		device->m_swapchainInfo.present_modes = present_modes;

		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_physicalDevice, instance->m_surface, &format_count, nullptr);
		eastl::vector<VkSurfaceFormatKHR> format_modes(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_physicalDevice, instance->m_surface, &format_count, format_modes.data());

		device->m_swapchainInfo.format_count = format_count;
		device->m_swapchainInfo.formats = format_modes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->m_physicalDevice, instance->m_surface, &device->m_swapchainInfo.surface_capabilities);


		eastl::vector<VkDeviceQueueCreateInfo> device_queue_infos(index_count);

		n = 0;
		float queue_priority = 1.0f;
		for (auto& i : device_queue_infos)
		{
			i.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			i.pQueuePriorities = &queue_priority;
			i.queueCount = 1;
			i.pNext = nullptr;
			i.queueFamilyIndex = indices[n];

			n++;
		}

		char* device_extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		phy_feat = {};
		phy_feat.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.queueCreateInfoCount = device_queue_infos.size();
		device_info.pQueueCreateInfos = device_queue_infos.data();
		device_info.pEnabledFeatures = &phy_feat;
		device_info.enabledLayerCount = 0;
		device_info.ppEnabledLayerNames = nullptr;
		device_info.enabledExtensionCount = 1;
		device_info.ppEnabledExtensionNames = device_extensions;

		VkResult device_created = vkCreateDevice(device->m_physicalDevice, &device_info, g_Vkhandle.m_alloc_callback, &device->m_device);

		TRC_TRACE("Obtaining Queues...");
		vkGetDeviceQueue(device->m_device, device->m_queues.graphics_queue, 0, &device->m_graphicsQueue);
		vkGetDeviceQueue(device->m_device, device->m_queues.present_queue, 0, &device->m_presentQueue);
		vkGetDeviceQueue(device->m_device, device->m_queues.transfer_queue, 0, &device->m_transferQueue);
		TRC_INFO("Queues Acquired");

		TRC_ASSERT(FindDepthFormat(device), "DepthFormat not found");

		VkCommandPoolCreateInfo graphics_pool_info = {};
		graphics_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphics_pool_info.queueFamilyIndex = device->m_queues.graphics_queue;
		graphics_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_ASSERT(vkCreateCommandPool(device->m_device, &graphics_pool_info, instance->m_alloc_callback, &device->m_graphicsCommandPool));
		TRC_INFO("Graphics command pool created");

		return device_created;
	}

	void _DestoryDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance)
	{
		vkDestroyCommandPool(device->m_device, device->m_graphicsCommandPool, instance->m_alloc_callback);

		vkDestroyDevice(device->m_device, instance->m_alloc_callback);
	}

	VkResult _CreateSurface(trace::VKHandle* instance)
	{
		VkResult res = VK_ERROR_INITIALIZATION_FAILED;

		switch (trace::Platform::s_api)
		{
		case trace::PlatformAPI::WINDOWS:
		{

			VkWin32SurfaceCreateInfoKHR create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.hinstance = (HINSTANCE)trace::Platform::GetAppHandle();
			create_info.hwnd = (HWND)trace::Application::s_instance->GetWindow()->GetNativeHandle();

			res = vkCreateWin32SurfaceKHR(instance->m_instance, &create_info, instance->m_alloc_callback, &instance->m_surface);

			break;
		}

		default:
		{
			TRC_ASSERT(false, " Platform type has to be specified ");
		}

		}

		return res;
	}

	void _DestorySurface(trace::VKHandle* instance)
	{

		vkDestroySurfaceKHR(instance->m_instance, instance->m_surface, instance->m_alloc_callback);

	}

	VkPhysicalDevice GetPhysicalDevice(trace::VKHandle* instance)
	{
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance->m_instance, &device_count, nullptr);
		eastl::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance->m_instance, &device_count, devices.data());

		eastl::vector<PhyScr> device_board;


		for (uint32_t i = 0; i < devices.size(); i++)
		{
			device_board.push_back({ devices[i], 0 });
		}

		PhyScr ret = { device_board[0].phy_device, device_board[0].score };
		for (auto& i : device_board)
		{
			RatePhysicalDevice(i.phy_device, i.score);
			if (i.score > ret.score)
			{
				ret = i;
			}
		}

		
		return ret.phy_device;
	}

	void RatePhysicalDevice(VkPhysicalDevice phy_device, uint32_t& score)
	{
		
		VkPhysicalDeviceProperties phy_prop;
		VkPhysicalDeviceFeatures phy_feat;
		vkGetPhysicalDeviceProperties(phy_device, &phy_prop);
		vkGetPhysicalDeviceFeatures(phy_device, &phy_feat);

		switch (phy_prop.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		{
			score += 100;
			break;
		}

		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		{
			score += 50;
			break;
		}

		case VK_PHYSICAL_DEVICE_TYPE_CPU:
		{
			score += 25;
			break;
		}

		}

		if (phy_feat.geometryShader)
		{
			score += 25;
		}
		if (phy_feat.tessellationShader)
		{
			score += 25;
		}

		score += phy_prop.limits.maxImageDimension2D;


		uint32_t queue_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_count, nullptr);
		eastl::vector<VkQueueFamilyProperties> queue_props(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_count, queue_props.data());

		for (auto& i : queue_props)
		{
			score += i.queueCount;
			if (i.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				score += 15;
			}

			if (i.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				score += 15;
			}

			if (i.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				score += 15;
			}

		}


		return;
	}

	VkResult _CreateSwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, uint32_t width, uint32_t height)
	{
		
		VkExtent2D extent = { width, height };
		//device->frames_in_flight = 2;


		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
		for (auto& i : device->m_swapchainInfo.present_modes)
		{
			if (i == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				present_mode = i;
			}
		}

		VkSurfaceFormatKHR surface_format = {};
		bool surface_found = false;
		for (auto& i : device->m_swapchainInfo.formats)
		{
			if (i.format == VK_FORMAT_R8G8B8_SRGB && i.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surface_found = true;
				surface_format = i;
			}
		}

		if (!surface_found)
		{
			surface_format = device->m_swapchainInfo.formats[0];
		}
		swapchain->m_format = surface_format;

		uint32_t image_count = device->m_swapchainInfo.surface_capabilities.minImageCount + 1;
		if (device->m_swapchainInfo.surface_capabilities.maxImageCount > 0 && image_count > device->m_swapchainInfo.surface_capabilities.maxImageCount)
		{
			image_count = device->m_swapchainInfo.surface_capabilities.maxImageCount;
		}


		VkExtent2D min = device->m_swapchainInfo.surface_capabilities.minImageExtent;
		VkExtent2D max = device->m_swapchainInfo.surface_capabilities.maxImageExtent;
		extent.width = TRC_CLAMP(extent.width, min.width, max.width);
		extent.height = TRC_CLAMP(extent.height, min.height, max.height);

		VkSwapchainCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.imageFormat = surface_format.format;
		create_info.surface = instance->m_surface;
		create_info.minImageCount = image_count;
		create_info.presentMode = present_mode;
		create_info.pNext = nullptr;
		create_info.oldSwapchain = VK_NULL_HANDLE; // Check vulkan docs for more info;
		create_info.clipped = VK_TRUE;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.preTransform = device->m_swapchainInfo.surface_capabilities.currentTransform;

		if (device->m_queues.graphics_queue != device->m_queues.present_queue)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			uint32_t queues[] = { device->m_queues.graphics_queue, device->m_queues.present_queue };
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queues;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = 0;
		}

		VkResult swapchain_create_result = vkCreateSwapchainKHR(device->m_device, &create_info, instance->m_alloc_callback, &swapchain->m_handle);
		VK_ASSERT(swapchain_create_result);

		device->m_frameBufferWidth = width;
		device->m_frameBufferHeight = height;

		TRC_TRACE("Obtaining Vulkan swapchain images...");
		image_count = 0;
		vkGetSwapchainImagesKHR(device->m_device, swapchain->m_handle, &image_count, nullptr);
		eastl::vector<VkImage> swap_images(image_count);
		vkGetSwapchainImagesKHR(device->m_device, swapchain->m_handle, &image_count, swap_images.data());

		swapchain->image_count = image_count;
		swapchain->m_images = swap_images;
		TRC_INFO("Vulkan swapchain images obtained");

		TRC_TRACE("Creating Vulkan Swapchain image views...");
		swapchain->m_imageViews.resize(image_count);
		int n = 0;
		for (auto& i : swapchain->m_images)
		{
			_CreateImageView(instance, device, &swapchain->m_imageViews[n], swapchain->m_format.format, VK_IMAGE_ASPECT_COLOR_BIT, &swapchain->m_images[n]);
			n++;
		}
		TRC_INFO("Vulkan Swapchain image views created");

		_CreateImage(
			instance,
			device,
			&swapchain->m_depthimage,
			device->m_depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			extent.width,
			extent.height
		);

		_CreateImageView(instance, device, &swapchain->m_depthimage.m_view, device->m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &swapchain->m_depthimage.m_handle);

		return swapchain_create_result;
	}

	void _DestroySwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain)
	{
		vkDeviceWaitIdle(device->m_device);
		_DestroyImage(instance, device, &swapchain->m_depthimage);

		for (auto& i : swapchain->m_imageViews)
		{
			_DestroyImageView(instance, device, &i);
		}

		vkDestroySwapchainKHR(device->m_device, swapchain->m_handle, instance->m_alloc_callback);
	}

	VkResult _RecreateSwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, uint32_t width, uint32_t height)
	{
		_DestroySwapchain(instance, device, swapchain);
		return _CreateSwapchain(instance, device, swapchain, width, height);
	}

	bool _AcquireSwapchainImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, VkSemaphore image_acquired_sem, trace::VKFence* fence, uint32_t* image_index, uint32_t timeout_ns)
	{
		VkFence _fence = VK_NULL_HANDLE;
		if (fence)
		{
			_fence = fence->m_handle;
		}

		VkResult result = vkAcquireNextImageKHR(device->m_device, swapchain->m_handle, timeout_ns, image_acquired_sem, _fence, image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			_RecreateSwapchain(instance, device, swapchain, device->m_frameBufferWidth, device->m_frameBufferHeight);
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			TRC_CRITICAL(" Failed to acquire swapchain image ");
			return false;
		}

		return true;
	}

	void _PresentSwapchainImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, VkQueue graphics_queue, VkQueue present_queue, VkSemaphore render_complete, uint32_t* present_image_index)
	{

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain->m_handle;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &render_complete;
		present_info.pImageIndices = present_image_index;
		present_info.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(present_queue, &present_info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			_RecreateSwapchain(instance, device, swapchain, device->m_frameBufferWidth, device->m_frameBufferHeight);
		}
		else if (result != VK_SUCCESS)
		{
			TRC_CRITICAL("Failed to present swapchain image");
		}

		return;
	}

	VkResult _CreateImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* out_image, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_prop, VkImageAspectFlags aspect_flags, uint32_t width, uint32_t height)
	{
		VkImageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.imageType = VK_IMAGE_TYPE_2D;
		create_info.extent.width = width;
		create_info.extent.height = height;
		create_info.extent.depth = 1;
		create_info.format = format;
		create_info.tiling = tiling;
		create_info.arrayLayers = 1;	// TODO: Configurable
		create_info.mipLevels = 4;	// TODO: Configurable
		create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.usage = usage;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		

		VkResult result = vkCreateImage(device->m_device, &create_info, instance->m_alloc_callback, &out_image->m_handle);
		VK_ASSERT(result);

		out_image->m_width = width;
		out_image->m_height = height;

		VkMemoryRequirements mem_req;
		vkGetImageMemoryRequirements(device->m_device, out_image->m_handle, &mem_req);

		uint32_t heap_index = FindMemoryIndex(device, mem_req.memoryTypeBits, memory_prop);
		if (heap_index == -1)
		{
			TRC_ERROR("Required memory type not found, vulkan image not valid");
		}

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_req.size;
		alloc_info.memoryTypeIndex = heap_index;

		VK_ASSERT(vkAllocateMemory(device->m_device, &alloc_info, instance->m_alloc_callback, &out_image->m_mem));

		vkBindImageMemory(device->m_device, out_image->m_handle, out_image->m_mem, 0); // TODO : Configurable Offset

		return result;
	}

	VkResult _CreateImageView(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkImageView* out_image_view, VkFormat format, VkImageAspectFlags aspect_flags, VkImage* image)
	{
		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.format = format;
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Configurable view type
		create_info.image = (*image);
		create_info.subresourceRange.aspectMask = aspect_flags;
		create_info.subresourceRange.baseArrayLayer = 0; // TODO: Configurable array layer
		create_info.subresourceRange.baseMipLevel = 0; // TODO: Configurable
		create_info.subresourceRange.layerCount = 1; // TODO: Configurable
		create_info.subresourceRange.levelCount = 1; // TODO: Configurable

		VkResult result = (vkCreateImageView(device->m_device, &create_info, instance->m_alloc_callback, out_image_view));

		VK_ASSERT(result);

		return result;
	}

	void _DestroyImageView(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkImageView* image_view)
	{
		VkImageView& view = (*image_view);
		if (view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device->m_device, view, instance->m_alloc_callback);
			view = VK_NULL_HANDLE;
			return;
		}

		TRC_ERROR("Vulkan Image is null : Can't destory invaild image");
		return;
	}

	void _DestroyImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* image)
	{
		if (image->m_handle != VK_NULL_HANDLE)
		{
			vkFreeMemory(device->m_device, image->m_mem, instance->m_alloc_callback);
			image->m_width = 0;
			image->m_height = 0;
			if (image->m_view != VK_NULL_HANDLE)
			{
				_DestroyImageView(instance, device, &image->m_view);
			}
			vkDestroyImage(device->m_device, image->m_handle, instance->m_alloc_callback);
			image->m_handle = VK_NULL_HANDLE;
		}
	}

	void _CopyBufferToImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer, trace::VKImage* image, trace::VKBuffer* buffer)
	{

		VkBufferImageCopy copy = {};
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;

		copy.imageOffset = { 0 };
		copy.imageExtent.depth = 1;

		copy.imageExtent.width = image->m_width;
		copy.imageExtent.height = image->m_height;

		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.mipLevel = 0;
		copy.imageSubresource.layerCount = 1;

		

		vkCmdCopyBufferToImage(
			command_buffer->m_handle,
			buffer->m_handle,
			image->m_handle,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copy
		);


		

	}

	bool _TransitionImageLayout(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer,trace::VKImage* image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
	{

		VkImageMemoryBarrier image_barrier = {};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.image = image->m_handle;
		image_barrier.oldLayout = old_layout;
		image_barrier.newLayout = new_layout;
		image_barrier.srcQueueFamilyIndex = device->m_queues.graphics_queue;
		image_barrier.dstQueueFamilyIndex = device->m_queues.graphics_queue;

		image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_barrier.subresourceRange.baseArrayLayer = 0;
		image_barrier.subresourceRange.baseMipLevel = 0;
		image_barrier.subresourceRange.layerCount = 1;
		image_barrier.subresourceRange.levelCount = 1;

		VkPipelineStageFlags src_stage_flag;
		VkPipelineStageFlags dst_stage_flag;

		if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_barrier.srcAccessMask = 0;
			image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			//TODO Check docs for more info
			src_stage_flag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dst_stage_flag = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//TODO Check docs for more info
			src_stage_flag = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst_stage_flag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			TRC_ERROR("Image tranfer layout not yet supported");
			return false;
		}

		vkCmdPipelineBarrier(
			command_buffer->m_handle,
			src_stage_flag,
			dst_stage_flag,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&image_barrier
		);

		return true;
	}

	VkResult _CreateSampler(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::TextureDesc& desc, VkSampler& sampler)
	{
		VkResult result;

		VkSamplerCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		create_info.addressModeU = convertAddressMode(desc.m_addressModeU);
		create_info.addressModeV = convertAddressMode(desc.m_addressModeV);
		create_info.addressModeW = convertAddressMode(desc.m_addressModeW);
		create_info.anisotropyEnable = VK_TRUE;
		create_info.maxAnisotropy = 16;
		create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		create_info.unnormalizedCoordinates = VK_FALSE;
		create_info.minFilter = convertFilter(desc.m_minFilterMode);
		create_info.magFilter = convertFilter(desc.m_magFilterMode);
		/// TODO Check Docs for more info
		create_info.compareEnable = VK_FALSE;
		create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		create_info.maxLod = 0.0f;
		create_info.minLod = 0.0f;
		create_info.mipLodBias = 0.0f;
		create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		result = vkCreateSampler(
			device->m_device,
			&create_info,
			instance->m_alloc_callback,
			&sampler
		);

		if (result != VK_SUCCESS)
		{
			TRC_ERROR("Unable to create sampler");
		}

		return result;
	}

	void _DestroySampler(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkSampler& sampler)
	{

		if (sampler)
		{
			vkDestroySampler(device->m_device, sampler, instance->m_alloc_callback);
		}

	}

	uint32_t FindMemoryIndex(trace::VKDeviceHandle* device, uint32_t memory_type, VkMemoryPropertyFlags mem_prop)
	{
		VkPhysicalDeviceMemoryProperties phy_mem_prop;
		vkGetPhysicalDeviceMemoryProperties(device->m_physicalDevice, &phy_mem_prop);

		for (uint32_t i = 0; i < phy_mem_prop.memoryTypeCount; i++)
		{
			if (memory_type & (1 << i) && (phy_mem_prop.memoryTypes[i].propertyFlags & mem_prop) == mem_prop)
			{
				return i;
			}
		}

		return -1;
	}

	void _AllocateCommandBuffer(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* out_command_buffer, bool is_primary)
	{

		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = command_pool;
		alloc_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		alloc_info.commandBufferCount = 1;

		VK_ASSERT(vkAllocateCommandBuffers(device->m_device, &alloc_info, &out_command_buffer->m_handle));
		out_command_buffer->m_state = trace::CommandBufferState::COMMAND_READY;

	}


	void _FreeCommandBuffer(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* command_buffer)
	{
		vkFreeCommandBuffers(device->m_device, command_pool, 1, &command_buffer->m_handle);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_NOT_ALLOCATED;
		command_buffer->m_handle = VK_NULL_HANDLE;

	}

	void _BeginCommandBuffer(trace::VKCommmandBuffer* command_buffer, trace::CommandBufferUsage usage)
	{

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_NO_FLAGS;

		switch (usage)
		{
		case trace::CommandBufferUsage::SINGLE_USE:
		{
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			break;
		}

		case trace::CommandBufferUsage::RENDER_PASS_CONTINUE:
		{
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			break;
		}

		case trace::CommandBufferUsage::SIMULTANEOUS_USE:
		{
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			break;
		}

		}


		vkBeginCommandBuffer(command_buffer->m_handle, &begin_info);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_RECORDING;

	}

	void _BeginCommandBufferSingleUse(trace::VKDeviceHandle* device,VkCommandPool command_pool ,trace::VKCommmandBuffer* command_buffer)
	{
		trace::CommandBufferUsage usage = trace::CommandBufferUsage::SINGLE_USE;
		_AllocateCommandBuffer(device, command_pool, command_buffer);
		_BeginCommandBuffer(command_buffer, usage);

	}

	void _EndCommandBuffer(trace::VKCommmandBuffer* command_buffer)
	{
		vkEndCommandBuffer(command_buffer->m_handle);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_RECORDING_ENDED;
	}

	void _EndCommandBufferSingleUse(trace::VKDeviceHandle* device, VkCommandPool command_pool, VkQueue queue, trace::VKCommmandBuffer* command_buffer)
	{

		_EndCommandBuffer(command_buffer);
		
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer->m_handle;

		VK_ASSERT(vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));

		VK_ASSERT(vkQueueWaitIdle(queue));

		_FreeCommandBuffer(device, command_pool, command_buffer);

	}

	void _CommandBufferSubmitted(trace::VKCommmandBuffer* command_buffer)
	{
		command_buffer->m_state = trace::CommandBufferState::COMMAND_SUBMMITED;
	}

	void _CommandBuffer_Reset(trace::VKCommmandBuffer* command_buffer)
	{
		vkResetCommandBuffer(command_buffer->m_handle, 0);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_READY;
	}

	void _CreateCommandBuffers(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkCommandPool command_pool, eastl::vector<trace::VKCommmandBuffer>& buffers)
	{

		if (device->m_graphicsCommandBuffers.empty())
		{
			buffers.resize(device->m_swapChain.image_count);

			for (uint32_t i = 0; i < device->m_swapChain.image_count; i++)
			{
				_AllocateCommandBuffer(device, command_pool, &buffers[i]);
			}
		}
		else
		{
			device->m_graphicsCommandBuffers.clear();
			buffers.resize(device->m_swapChain.image_count);

			for (uint32_t i = 0; i < device->m_swapChain.image_count; i++)
			{
				_AllocateCommandBuffer(device, command_pool, &buffers[i]);
			}
		}

	}

	VkResult _CreateFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer, eastl::vector<VkImageView> attachments, trace::VKRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t attachment_count)
	{
		VkResult result;

		frame_buffer->m_attachments.resize(attachment_count);
		for (uint32_t i = 0; i < attachment_count; i++)
		{
			frame_buffer->m_attachments[i] = attachments[i];
		}


		VkFramebufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.renderPass = render_pass->m_handle;
		create_info.layers = 1;
		create_info.attachmentCount = attachment_count;
		create_info.height = height;
		create_info.width = width;
		create_info.pAttachments = frame_buffer->m_attachments.data();
		
		
		result = vkCreateFramebuffer(device->m_device, &create_info, instance->m_alloc_callback, &frame_buffer->m_handle);

		VK_ASSERT(result);
		frame_buffer->m_height = height;
		frame_buffer->m_width = width;
		frame_buffer->m_attachmentCount = attachment_count;

		return result;
	}

	void _DestoryFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer)
	{
		vkDestroyFramebuffer(device->m_device, frame_buffer->m_handle, instance->m_alloc_callback);
	}

	void _RegenerateFrameBuffers(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, trace::VKRenderPass* render_pass)
	{
		swapchain->m_frameBuffers.resize(swapchain->image_count);
		for (uint32_t i = 0; i < swapchain->image_count; i++)
		{

			uint32_t attachment_count = 2;
			eastl::vector<VkImageView> attachments = {
				swapchain->m_imageViews[i],
				swapchain->m_depthimage.m_view
			};

			_CreateFrameBuffer(instance, device, &swapchain->m_frameBuffers[i], attachments, &device->m_renderPass, device->m_frameBufferWidth, device->m_frameBufferHeight, attachment_count);

		}
	}

	void _CreateFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence, bool signaled)
	{

		VkFenceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		if (signaled)
		{
			create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			out_fence->m_isSignaled = true;
		}

		VK_ASSERT(vkCreateFence(device->m_device, &create_info, instance->m_alloc_callback, &out_fence->m_handle));

	}

	bool _WaitFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence, uint64_t timeout_ns)
	{
		if (out_fence->m_isSignaled)
			return true;

		VkResult result = vkWaitForFences(device->m_device, 1, &out_fence->m_handle, VK_TRUE, timeout_ns);

		switch (result)
		{
		case VK_SUCCESS:
		{
			out_fence->m_isSignaled = true;
			return true;
		}
		case VK_TIMEOUT:
		{
			TRC_WARN("Fence timeout");
			break;
		}
		// resolve for other response
		}

		return false;
	}

	void _DestroyFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence)
	{
		if (out_fence->m_handle)
		{
			vkDestroyFence(device->m_device, out_fence->m_handle, instance->m_alloc_callback);
			return;
		}
		TRC_ERROR("Can't destory an invaild fence line: %d \n ==>Function: %s", __LINE__, __FUNCTION__);
	}

	void _FenceReset(trace::VKDeviceHandle* device, trace::VKFence* fence)
	{
		fence->m_isSignaled = false;
		vkResetFences(device->m_device, 1, &fence->m_handle);
	}

	VkResult _CreateRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass, glm::vec4 clear_color, glm::vec4 render_area, float depth_value, uint32_t stencil_value, trace::VKSwapChain* swapchain)
	{
		VkResult result = VK_ERROR_INITIALIZATION_FAILED;

		uint32_t attachment_count = 2; // TODO: Configurable
		eastl::vector<VkAttachmentDescription> attachments;
		attachments.resize(attachment_count);

		VkAttachmentDescription color_attachment = {};

		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
		color_attachment.format = swapchain->m_format.format;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//TODO: Check why values can't be assigned to by indexing
		attachments[0] = color_attachment;

		VkAttachmentReference color_attach_ref = {};
		color_attach_ref.attachment = 0;
		color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment = {};

		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
		depth_attachment.format = device->m_depthFormat;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		
		//TODO: Check why values can't be assigned to by indexing
		attachments[1] = (depth_attachment);

		VkAttachmentReference depth_attach_ref = {};
		depth_attach_ref.attachment = 1;
		depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// TODO: other attachent { resolve, input, preserve }

		VkSubpassDescription subpass_info = {};
		subpass_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_info.colorAttachmentCount = 1;
		subpass_info.pColorAttachments = &color_attach_ref;
		subpass_info.pDepthStencilAttachment = &depth_attach_ref;

		// TODO: Configurable
		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass = 0;
		subpass_dependency.srcAccessMask = 0;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dependencyFlags = 0;


		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &subpass_dependency;
		render_pass_info.attachmentCount = attachment_count;
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_info;

		result = vkCreateRenderPass(device->m_device, &render_pass_info, instance->m_alloc_callback, &render_pass->m_handle);

		VK_ASSERT(result);
		render_pass->clear_color = clear_color;
		render_pass->depth_value = depth_value;
		render_pass->stencil_value = stencil_value;
		render_pass->render_area = render_area;


		return result;
	}

	void _DestroyRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass)
	{
		vkDestroyRenderPass(device->m_device, render_pass->m_handle, instance->m_alloc_callback);
	}

	void _BeginRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass, trace::VKCommmandBuffer* command_buffer, VkFramebuffer framebuffer)
	{

		VkRenderPassBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin_info.renderPass = render_pass->m_handle;
		begin_info.renderArea.offset.x = render_pass->render_area.x;
		begin_info.renderArea.offset.y = render_pass->render_area.y;
		begin_info.renderArea.extent.width = render_pass->render_area.z;
		begin_info.renderArea.extent.height = render_pass->render_area.w;
		begin_info.framebuffer = framebuffer;
		
		VkClearValue clear_colors[2] = {};
		clear_colors[0].color.float32[0] = render_pass->clear_color.r;
		clear_colors[0].color.float32[1] = render_pass->clear_color.g;
		clear_colors[0].color.float32[2] = render_pass->clear_color.b;
		clear_colors[0].color.float32[3] = render_pass->clear_color.a;

		clear_colors[1].depthStencil.depth = render_pass->depth_value;
		clear_colors[1].depthStencil.stencil = render_pass->stencil_value;

		begin_info.clearValueCount = 2;
		begin_info.pClearValues = clear_colors;

		vkCmdBeginRenderPass(command_buffer->m_handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_IN_RENDER_PASS;
		


	}

	void _EndRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer)
	{
		vkCmdEndRenderPass(command_buffer->m_handle);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_RECORDING;
	}

	VkResult _CreatePipeline(trace::VKHandle* instance, trace::VKDeviceHandle* device, uint32_t view_port_count, VkViewport* view_ports, uint32_t scissor_count, VkRect2D* scissors, trace::PipelineStateDesc desc, trace::VKPipeline* out_pipeline)
	{
		VkResult result = VK_ERROR_INITIALIZATION_FAILED;

		eastl::vector<VkPipelineShaderStageCreateInfo> shader_stages;

		if (desc.vertex_shader != nullptr)
		{
			trace::VulkanShader* shader = reinterpret_cast<trace::VulkanShader*>(desc.vertex_shader);
			shader_stages.push_back(shader->m_handle.create_info);
		}
		if (desc.pixel_shader != nullptr)
		{
			trace::VulkanShader* shader = reinterpret_cast<trace::VulkanShader*>(desc.pixel_shader);
			shader_stages.push_back(shader->m_handle.create_info);
		}

		VkVertexInputBindingDescription binding;
		eastl::vector<VkVertexInputAttributeDescription> attrs;

		parseInputLayout(desc.input_layout, binding, attrs);
	
		VkPipelineVertexInputStateCreateInfo vert_info = {};
		vert_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_info.vertexBindingDescriptionCount = 1; // TODO
		vert_info.pVertexBindingDescriptions = &binding;
		vert_info.vertexAttributeDescriptionCount = attrs.size();
		vert_info.pVertexAttributeDescriptions = attrs.data();


		VkPipelineInputAssemblyStateCreateInfo input_info = {};
		input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_info.primitiveRestartEnable = VK_FALSE; // TODO
		input_info.topology = convertTopology(desc.topology);
		

		VkPipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = view_port_count;
		viewport_info.pViewports = view_ports;
		viewport_info.scissorCount = scissor_count;
		viewport_info.pScissors = scissors;


		VkPipelineRasterizationStateCreateInfo raster_info = {};
		raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		parseRasterizerState( desc.rateriser_state,raster_info);

		VkPipelineMultisampleStateCreateInfo multi_info = {};
		multi_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		parseMultiState(multi_info);

		/*
		// TODO: Implement tesallation state
		VkPipelineTessellationStateCreateInfo tess_info = {};
		tess_info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		*/

		// =============== TODO========
		uint32_t dynamic_state_count = 3;

		VkDynamicState dyn_states[] =
		{
			VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_VIEWPORT
		};
		//===============================

		VkPipelineDynamicStateCreateInfo dynamic_info = {};
		dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_info.dynamicStateCount = dynamic_state_count;
		dynamic_info.pDynamicStates = dyn_states;

		VkPipelineDepthStencilStateCreateInfo depth_sten_info = {};
		depth_sten_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		parseDepthStenState(desc.depth_sten_state, depth_sten_info);


		VkPipelineColorBlendAttachmentState attach_state;
		VkPipelineColorBlendStateCreateInfo color_blend_info = {};
		color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		parseColorBlendState(desc.blend_state, color_blend_info, attach_state);

		// TODO: complete pipeline layout creation info
		VkPipelineLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		parsePipelineLayout(instance, device, desc, layout_info, out_pipeline);
		
		result = vkCreatePipelineLayout(
			device->m_device,
			&layout_info,
			instance->m_alloc_callback,
			&out_pipeline->m_layout
		);

		VK_ASSERT(result);

		VkGraphicsPipelineCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		create_info.pColorBlendState = &color_blend_info;
		create_info.pDepthStencilState = &depth_sten_info;
		create_info.pDynamicState = &dynamic_info;
		create_info.pInputAssemblyState = &input_info;
		create_info.pMultisampleState = &multi_info;
		create_info.pRasterizationState = &raster_info;
		create_info.pTessellationState = nullptr;
		create_info.pVertexInputState = &vert_info;
		create_info.pViewportState = &viewport_info;
		create_info.renderPass = device->m_renderPass.m_handle; // TODO: Maybe can be dynamic
		create_info.basePipelineHandle = VK_NULL_HANDLE; // TODO:  Check Docs
		create_info.basePipelineIndex = -1; // TODO
		create_info.stageCount = shader_stages.size();
		create_info.pStages = shader_stages.data();
		create_info.subpass = 0; // TODO
		create_info.layout = out_pipeline->m_layout;

		result = vkCreateGraphicsPipelines(
			device->m_device,
			VK_NULL_HANDLE, // TODO: Check Docs for more info
			1,
			&create_info,
			instance->m_alloc_callback,
			&out_pipeline->m_handle
			);

		VK_ASSERT(result);

		return result;
	}

	void _DestroyPipeline(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKPipeline* pipeline)
	{
		for (uint32_t i = 0; i < 16; i++)
		{
			if (pipeline->Scene_buffers[i].m_handle != VK_NULL_HANDLE)
			{
				_DestoryBuffer(instance, device, &pipeline->Scene_buffers[i]);
			}
		}

		if (pipeline->Scene_layout)
		{
			vkDestroyDescriptorSetLayout(device->m_device, pipeline->Scene_layout, instance->m_alloc_callback);
			pipeline->Scene_layout = VK_NULL_HANDLE;
		}

		if (pipeline->Scene_pool)
		{
			vkDestroyDescriptorPool(device->m_device, pipeline->Scene_pool, instance->m_alloc_callback);
			pipeline->Scene_pool = VK_NULL_HANDLE;
		}

		if (pipeline->m_layout)
		{
			vkDestroyPipelineLayout(device->m_device, pipeline->m_layout, instance->m_alloc_callback);
			if (pipeline->m_handle)
			{
				vkDestroyPipeline(device->m_device, pipeline->m_handle, instance->m_alloc_callback);
				pipeline->m_handle = VK_NULL_HANDLE;
			}

			pipeline->m_layout = VK_NULL_HANDLE;
		}
		

	}

	VkResult _CreateShader(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKShader* out_shader, trace::ShaderStage stage, std::vector<uint32_t>& code)
	{
		VkResult result;

		out_shader->m_code = code;

		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = out_shader->m_code.size() * sizeof(uint32_t);
		create_info.pCode = out_shader->m_code.data();

		result = vkCreateShaderModule(device->m_device, &create_info, instance->m_alloc_callback, &out_shader->m_module);


		VK_ASSERT(result);

		out_shader->create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		out_shader->create_info.module = out_shader->m_module;
		out_shader->create_info.pName = "main";
		out_shader->create_info.stage = convertShaderStage(stage);
		out_shader->create_info.pSpecializationInfo = nullptr; // TODO: Check Docs for more info


		return result;
	}

	void _DestoryShader(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKShader* shader)
	{
		if (shader->m_module)
		{
			vkDestroyShaderModule(device->m_device, shader->m_module, instance->m_alloc_callback);
		}
	}

	VkResult _CreateBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* out_buffer, trace::BufferInfo buffer_info)
	{
		VkResult result;

		auto usage = convertBufferUsage(buffer_info.m_usage);

		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Check Docs for more info
		create_info.size = buffer_info.m_size;
		create_info.usage = usage.first;

		result = vkCreateBuffer(device->m_device, &create_info, instance->m_alloc_callback, &out_buffer->m_handle);

		VK_ASSERT(result);

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(device->m_device, out_buffer->m_handle, &mem_requirements);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = FindMemoryIndex(device, mem_requirements.memoryTypeBits, usage.second);

		if (vkAllocateMemory(device->m_device, &alloc_info, instance->m_alloc_callback, &out_buffer->m_memory) != VK_SUCCESS)
		{
			TRC_ERROR("failed to allocate buffer memory");
		}

		_BindBufferMem(instance, device, out_buffer->m_handle, out_buffer->m_memory, 0);


		return result;
	}

	void _DestoryBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* buffer)
	{

		if (buffer->m_memory)
		{
			vkFreeMemory(device->m_device, buffer->m_memory, instance->m_alloc_callback);
			buffer->m_memory = VK_NULL_HANDLE;
		}

		if (buffer->m_handle)
		{
			vkDestroyBuffer(device->m_device, buffer->m_handle, instance->m_alloc_callback);
			buffer->m_handle = VK_NULL_HANDLE;
		}

	}

	void _BindBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* buffer)
	{
	}

	void _BindBufferMem(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkBuffer buffer, VkDeviceMemory device_mem, uint32_t offset)
	{
		vkBindBufferMemory(device->m_device, buffer, device_mem, offset);
	}

	void _MapMemory(trace::VKDeviceHandle* device, void* data, VkDeviceMemory memory, uint32_t offset, uint32_t size, uint32_t flags)
	{

		vkMapMemory(device->m_device, memory, offset, size, flags, &data);


	}

	void _UnMapMemory(trace::VKDeviceHandle* device, VkDeviceMemory memory)
	{
		vkUnmapMemory(device->m_device, memory);
	}

	void _CopyBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* src, trace::VKBuffer* dst, uint32_t size, uint32_t offset)
	{
		trace::VKCommmandBuffer cmd_buf;
		_BeginCommandBufferSingleUse(device, device->m_graphicsCommandPool, &cmd_buf);

		VkBufferCopy copy_region = {};
		copy_region.size = size;
		copy_region.dstOffset = 0;
		copy_region.srcOffset = offset;

		vkCmdCopyBuffer(cmd_buf.m_handle, src->m_handle, dst->m_handle, 1, &copy_region);

		_EndCommandBufferSingleUse(device, device->m_graphicsCommandPool, device->m_graphicsQueue, &cmd_buf);




	}

	void parseInputLayout(trace::InputLayout& layout, VkVertexInputBindingDescription& binding, eastl::vector<VkVertexInputAttributeDescription>& attrs)
	{
		VkPipelineVertexInputStateCreateInfo result;

		VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX;
		switch (layout.input_class)
		{
		case trace::InputClassification::PER_VERTEX_DATA:
		{
			input_rate = VK_VERTEX_INPUT_RATE_VERTEX;
			break;
		}
		case trace::InputClassification::PER_INSTANCE_DATA:
		{
			input_rate = VK_VERTEX_INPUT_RATE_INSTANCE;
			break;
		}
		}

		VkVertexInputBindingDescription& bind_desc = binding;
		bind_desc.binding = 0; // TODO
		bind_desc.stride = layout.stride;
		bind_desc.inputRate = input_rate;

		eastl::vector<VkVertexInputAttributeDescription>& attr_desc = attrs;

		for (trace::InputLayout::Element& elem : layout.elements)
		{
			VkVertexInputAttributeDescription _att = {};
			_att.binding = 0; // TODO
			_att.location = elem.index;
			_att.format = convertFmt(elem.format);
			_att.offset = elem.offset;

			attr_desc.push_back(_att);
		}

	}

	void parseRasterizerState(trace::RaterizerState& raterizer, VkPipelineRasterizationStateCreateInfo& create_info)
	{

		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		create_info.lineWidth = 1.0f; // TODO
		create_info.polygonMode = convertPolygonMode(raterizer.fill_mode);
		create_info.cullMode = raterizer.cull_mode == trace::CullMode::FRONT ? VK_CULL_MODE_FRONT_BIT : VK_CULL_MODE_BACK_BIT;
		create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // TODO
		create_info.rasterizerDiscardEnable = VK_FALSE; // TODO
		create_info.depthClampEnable = VK_FALSE; // TODO
		create_info.depthBiasEnable = VK_FALSE; // TODO
		create_info.depthBiasConstantFactor = 0.0f; // TODO
		create_info.depthBiasClamp = 0.0f; // TODO
		create_info.depthBiasSlopeFactor = 0.0f; // TODO



	}

	void parseMultiState(VkPipelineMultisampleStateCreateInfo& create_info)
	{
		// --------------- TODO ----------------------
		create_info.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		create_info.sampleShadingEnable = VK_FALSE;
		create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		create_info.minSampleShading = 1.0f; // Optional
		create_info.pSampleMask = nullptr; // Optional
		create_info.alphaToCoverageEnable = VK_FALSE; // Optional
		create_info.alphaToOneEnable = VK_FALSE;
		//--------------------------------------------

	}

	void parseDepthStenState(trace::DepthStencilState& state, VkPipelineDepthStencilStateCreateInfo& create_info)
	{

		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		create_info.depthTestEnable = state.depth_test_enable ? VK_TRUE : VK_FALSE;
		create_info.depthWriteEnable = state.depth_test_enable ? VK_TRUE : VK_FALSE;
		create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // TODO
		create_info.depthBoundsTestEnable = VK_FALSE; // TODO
		create_info.minDepthBounds = state.minDepth; // Check docs
		create_info.maxDepthBounds = state.maxDepth; // Chech docs
		create_info.stencilTestEnable = state.stencil_test_enable ? VK_TRUE : VK_FALSE;
		create_info.front = {}; // TODO
		create_info.back = {}; // TODO

	}

	void parseColorBlendState(trace::ColorBlendState& state, VkPipelineColorBlendStateCreateInfo& create_info, VkPipelineColorBlendAttachmentState& colorBlendAttachment)
	{

		// ------------------ TODO----------------------------------------
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; //
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		//------------------------------------------------------------------


		// Read Through docs and check tutorials
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		create_info.logicOpEnable = VK_FALSE;
		create_info.logicOp = VK_LOGIC_OP_COPY;
		create_info.attachmentCount = 1;
		create_info.pAttachments = &colorBlendAttachment;
		create_info.blendConstants[0] = 0.0f;
		create_info.blendConstants[1] = 0.0f;
		create_info.blendConstants[2] = 0.0f;
		create_info.blendConstants[3] = 0.0f;

	}

	void parsePipelineLayout(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::PipelineStateDesc& desc, VkPipelineLayoutCreateInfo& create_info, trace::VKPipeline* pipeline)
	{


		VkResult result;


		std::vector<VkDescriptorPoolSize> SceneGlobalData_poolSizes;
		std::vector<VkDescriptorSet> SceneGlobalData_sets;
		std::vector<VkDescriptorSetLayoutBinding> SceneGlobalData_bindings;
		uint32_t Scene_buffersSizes[16] = {};

		if (desc.vertex_shader != nullptr)
		{
			trace::VulkanShader* shader = reinterpret_cast<trace::VulkanShader*>(desc.vertex_shader);
			spirv_cross::Compiler compiler(shader->m_handle.m_code);

			auto resources = compiler.get_shader_resources();



			for (auto& resource : resources.uniform_buffers)
			{
				auto type = compiler.get_type(resource.type_id);
				size_t size = compiler.get_declared_struct_size(type);
				uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

				std::cout << "Name: " << resource.name << std::endl;
				std::cout << "Size: " << size << std::endl;
				std::cout << "Set: " << set << std::endl;
				std::cout << "Binding: " << binding << std::endl;

				VkDescriptorSetLayoutBinding bind = {};
				bind.binding = binding;
				bind.descriptorCount = 1;
				bind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorPoolSize pool_size = {};
				pool_size.descriptorCount = 1 * 3;
				pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

				if (!type.array.empty())
				{
					std::cout << "Array size: " << type.array[0] << std::endl;
					bind.descriptorCount = type.array[0];
					pool_size.descriptorCount = type.array[0] * 3;
					size = size * type.array[0];
				}


				switch (set)
				{
				case 0:
				{

					SceneGlobalData_bindings.push_back(bind);
					SceneGlobalData_poolSizes.push_back(pool_size);

					Scene_buffersSizes[bind.binding] = size;
					pipeline->Scene_bindings[bind.binding] = bind;
					break;
				}
				}

				std::cout << std::endl;

			}

			for (auto& resource : resources.sampled_images)
			{
				auto type = compiler.get_type(resource.type_id);
				uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

				std::cout << "Name: " << resource.name << std::endl;
				std::cout << "Set: " << set << std::endl;
				std::cout << "Binding: " << binding << std::endl;

				VkDescriptorSetLayoutBinding bind = {};
				bind.binding = binding;
				bind.descriptorCount = 1;
				bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorPoolSize pool_size = {};
				pool_size.descriptorCount = 1 * 3;
				pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

				if (!type.array.empty())
				{
					std::cout << "Array size: " << type.array[0] << std::endl;
					bind.descriptorCount = type.array[0];
					pool_size.descriptorCount = type.array[0] * 3;
				}


				switch (set)
				{
				case 0:
				{

					SceneGlobalData_bindings.push_back(bind);
					SceneGlobalData_poolSizes.push_back(pool_size);

					pipeline->Scene_bindings[bind.binding] = bind;
					break;
				}
				}

				std::cout << std::endl;

			}




		}


		if (desc.pixel_shader != nullptr)
		{
			trace::VulkanShader* shader = reinterpret_cast<trace::VulkanShader*>(desc.pixel_shader);
			spirv_cross::Compiler compiler(shader->m_handle.m_code);

			auto resources = compiler.get_shader_resources();


			for (auto& resource : resources.uniform_buffers)
			{
				auto type = compiler.get_type(resource.type_id);
				size_t size = compiler.get_declared_struct_size(type);
				uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

				std::cout << "Name: " << resource.name << std::endl;
				std::cout << "Size: " << size << std::endl;
				std::cout << "Set: " << set << std::endl;
				std::cout << "Binding: " << binding << std::endl;

				VkDescriptorSetLayoutBinding bind = {};
				bind.binding = binding;
				bind.descriptorCount = 1;
				bind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorPoolSize pool_size = {};
				pool_size.descriptorCount = 1 * 3;
				pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

				if (!type.array.empty())
				{
					std::cout << "Array size: " << type.array[0] << std::endl;
					bind.descriptorCount = type.array[0];
					pool_size.descriptorCount = type.array[0] * 3;
					size = size * type.array[0];
				}


				switch (set)
				{
				case 0:
				{

					SceneGlobalData_bindings.push_back(bind);
					SceneGlobalData_poolSizes.push_back(pool_size);

					Scene_buffersSizes[bind.binding] = size;
					pipeline->Scene_bindings[bind.binding] = bind;
					break;
				}
				}
				std::cout << std::endl;

			}

			for (auto& resource : resources.sampled_images)
			{
				auto type = compiler.get_type(resource.type_id);
				uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

				std::cout << "Name: " << resource.name << std::endl;
				std::cout << "Set: " << set << std::endl;
				std::cout << "Binding: " << binding << std::endl;

				VkDescriptorSetLayoutBinding bind = {};
				bind.binding = binding;
				bind.descriptorCount = 1;
				bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorPoolSize pool_size = {};
				pool_size.descriptorCount = 1 * 3;
				pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

				if (!type.array.empty())
				{
					std::cout << "Array size: " << type.array[0] << std::endl;
					bind.descriptorCount = type.array[0];
					pool_size.descriptorCount = type.array[0] * 3;
				}


				switch (set)
				{
				case 0:
				{

					SceneGlobalData_bindings.push_back(bind);
					SceneGlobalData_poolSizes.push_back(pool_size);

					pipeline->Scene_bindings[bind.binding] = bind;
					break;
				}
				}
				std::cout << std::endl;

			}



		}

		if (!SceneGlobalData_bindings.empty())
		{

			VkDescriptorSetLayoutCreateInfo _info = {};
			_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			_info.bindingCount = SceneGlobalData_bindings.size();
			_info.pBindings = SceneGlobalData_bindings.data();

			result = vkCreateDescriptorSetLayout(device->m_device, &_info, instance->m_alloc_callback, &pipeline->Scene_layout);

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.maxSets = 3;
			pool_info.poolSizeCount = SceneGlobalData_poolSizes.size();
			pool_info.pPoolSizes = SceneGlobalData_poolSizes.data();

			result = vkCreateDescriptorPool(device->m_device, &pool_info, instance->m_alloc_callback, &pipeline->Scene_pool);

			VkDescriptorSetLayout test[3] = {
				pipeline->Scene_layout,
				pipeline->Scene_layout,
				pipeline->Scene_layout
			};

			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = pipeline->Scene_pool;
			alloc_info.descriptorSetCount = 3;
			alloc_info.pSetLayouts = test;


			result = vkAllocateDescriptorSets(device->m_device, &alloc_info, pipeline->Scene_sets);

			for (uint32_t i = 0; i < SceneGlobalData_bindings.size(); i++)
			{
				VkDescriptorSetLayoutBinding& bind = SceneGlobalData_bindings[i];

				switch (bind.descriptorType)
				{

				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					createBuffer(instance, device, Scene_buffersSizes[bind.binding], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pipeline->Scene_buffers[bind.binding].m_handle, pipeline->Scene_buffers[bind.binding].m_memory);
					break;
				}

				}
			}

		}

		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &pipeline->Scene_layout;

	}

	VkFormat convertFmt(trace::Format format)
	{
		VkFormat result;

		switch (format)
		{
		case trace::Format::R32G32B32_FLOAT:
		{
			result = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		}
		case trace::Format::R32G32B32_UINT:
		{
			result = VK_FORMAT_R32G32B32_UINT;
			break;
		}
		case trace::Format::R32G32_FLOAT:
		{
			result = VK_FORMAT_R32G32_SFLOAT;
			break;
		}
		case trace::Format::R32G32_UINT:
		{
			result = VK_FORMAT_R32G32_UINT;
			break;
		}
		
		case trace::Format::R8G8B8A8_SNORM:
		{
			result = VK_FORMAT_R8G8B8A8_SNORM;
			break;
		}
		case trace::Format::R8G8B8A8_SRBG:
		{
			result = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		}
		case trace::Format::R8G8B8_SRBG:
		{
			result = VK_FORMAT_R8G8B8_SRGB;
			break;
		}
		case trace::Format::R8G8B8_SNORM:
		{
			result = VK_FORMAT_R8G8B8_SNORM;
			break;
		}
		}

		return result;
	}

	VkPrimitiveTopology convertTopology(trace::PrimitiveTopology topology)
	{
		VkPrimitiveTopology result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		switch (topology)
		{
		case trace::PrimitiveTopology::TRIANGLE_LIST:
		{
			result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		}
		case trace::PrimitiveTopology::TRIANGLE_STRIP:
		{
			result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		}
		}

		return result;
	}

	VkPolygonMode convertPolygonMode(trace::FillMode fillmode)
	{
		VkPolygonMode mode;

		switch (fillmode)
		{
		case trace::FillMode::SOLID:
		{
			mode = VK_POLYGON_MODE_FILL;
			break;
		}
		case trace::FillMode::WIREFRAME:
		{
			mode = VK_POLYGON_MODE_LINE;
			break;
		}
		}

		return mode;
	}

	VkShaderStageFlagBits convertShaderStage(trace::ShaderStage stage)
	{
		VkShaderStageFlagBits result;

		switch (stage)
		{
		case trace::ShaderStage::VERTEX_SHADER:
		{
			result = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		}
		case trace::ShaderStage::PIXEL_SHADER:
		{
			result = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		}
		}

		return result;
	}

	VkSamplerAddressMode convertAddressMode(trace::AddressMode address_mode)
	{
		VkSamplerAddressMode mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		switch (address_mode)
		{
		case trace::AddressMode::REPEAT:
		{
			mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		}
		
		case trace::AddressMode::MIRRORED_REPEAT:
		{
			mode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		}
		}

		return mode;
	}

	VkFilter convertFilter(trace::FilterMode filter)
	{
		VkFilter result = VK_FILTER_LINEAR;

		switch (filter)
		{
		case trace::FilterMode::LINEAR:
			{
				result = VK_FILTER_LINEAR;
				break;
			}
		}

		return result;
	}

	std::pair<VkBufferUsageFlags, VkMemoryPropertyFlags> convertBufferUsage(trace::BufferUsage usage)
	{
		std::pair<VkBufferUsageFlags, VkMemoryPropertyFlags> result;

		if (TRC_HAS_FLAG(usage, trace::BufferUsage::VERTEX_BUFFER))
		{
			result.first = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			result.second = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}

		if (TRC_HAS_FLAG(usage, trace::BufferUsage::INDEX_BUFFER))
		{
			result.first = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			result.second = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}

		if (TRC_HAS_FLAG(usage, trace::BufferUsage::STAGING_BUFFER))
		{
			result.first = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			result.second = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		return result;
	}

	bool _ResultIsSuccess(VkResult result)
	{


		switch (result)
		{
		case VK_SUCCESS:
		{
			return true;
		}
		}

		return false;
	}

	const char* _GetResultString(VkResult result)
	{
		switch (result)
		{
		case VK_SUCCESS:
		{
			return "Successful";
		}
		}
		return nullptr;
	}


	void createBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags prop_flags, VkBuffer& buffer, VkDeviceMemory& buffer_mem)
	{
		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.usage = usage_flags;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.size = size;

		if (vkCreateBuffer(device->m_device, &create_info, instance->m_alloc_callback, &buffer) != VK_SUCCESS)
		{
			TRC_ERROR(" Failed to create buffer ");
		}

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(device->m_device, buffer, &mem_requirements);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = FindMemoryIndex(device,mem_requirements.memoryTypeBits, prop_flags);

		if (vkAllocateMemory(device->m_device, &alloc_info, instance->m_alloc_callback, &buffer_mem) != VK_SUCCESS)
		{
			TRC_ERROR(" Unable to allocate buffer memory ");
		}

		vkBindBufferMemory(device->m_device, buffer, buffer_mem, 0);
	}

}