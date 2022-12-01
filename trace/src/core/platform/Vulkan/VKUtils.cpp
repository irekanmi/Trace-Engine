#include "pch.h"

#include "VkUtils.h"
#include "EASTL/vector.h"
#include "core/Platform.h"
#include "core/Application.h"
#include "vulkan/vulkan_win32.h"
#include "EASTL/map.h"

struct PhyScr
{
	VkPhysicalDevice phy_device;
	uint32_t score;
};

/* 

		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderpass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createDepthResources();
		createFramebuffers();
		createCommandPool();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createVertexBuffers();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSet();
		createCommandBuffer();
		createSyncObjects();

*/

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

extern trace::VkHandle g_Vkhandle;
extern trace::VkDeviceHandle g_VkDevice;

namespace vk {

	eastl::vector<const char*> g_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	eastl::vector<const char*> g_extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
	};



	VkResult _CreateInstance(trace::VkHandle* instance)
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
		const char** plat_ext = trace::Platform::GetExtensions(ext_count);

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

	void _DestroyInstance(trace::VkHandle* instance)
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

	void EnableValidationlayers(trace::VkHandle* instance)
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

		VkResult res = CreateDebugUtilsMessengerEXT(instance->m_instance, &create_info, instance->m_alloc_callback, &instance->m_debugutils);

		VK_ASSERT(res);

	}

	void DisableValidationlayers(trace::VkHandle* instance)
	{
		DestroyDebugUtilsMessengerEXT(instance->m_instance, instance->m_debugutils, instance->m_alloc_callback);
	}

	VkResult _CreateDevice(trace::VkDeviceHandle* device, trace::VkHandle* instance)
	{
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
		TRC_INFO("Queues Acquired")

		return device_created;
	}

	void _DestoryDevice(trace::VkDeviceHandle* device, trace::VkHandle* instance)
	{
		vkDestroyDevice(device->m_device, instance->m_alloc_callback);
	}

	VkResult _CreateSurface(trace::VkHandle* instance)
	{
		switch (trace::Platform::s_api)
		{
		case trace::PlatformAPI::WINDOWS:
		{

			VkWin32SurfaceCreateInfoKHR create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.hinstance = (HINSTANCE)trace::Platform::GetAppHandle();
			create_info.hwnd = (HWND)trace::Application::s_instance->GetWindow()->GetNativeHandle();

			return vkCreateWin32SurfaceKHR(instance->m_instance, &create_info, instance->m_alloc_callback, &instance->m_surface);

			break;
		}
		}

		TRC_ASSERT(false, " Platform type has to be specified ");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	void _DestorySurface(trace::VkHandle* instance)
	{

		vkDestroySurfaceKHR(instance->m_instance, instance->m_surface, instance->m_alloc_callback);

	}

	VkPhysicalDevice GetPhysicalDevice(trace::VkHandle* instance)
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
		score;
		
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

	VkResult _CreateSwapchain(trace::VkHandle* instance, trace::VkDeviceHandle* device, trace::VkSwapChain* swapchain, uint32_t width, uint32_t height)
	{
		VkExtent2D extent = { width, height };
		swapchain->frames_in_flight = 2;


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
		swapchain->image_count = image_count;


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


		return swapchain_create_result;
	}

	void _DestroySwapchain(trace::VkHandle* instance, trace::VkDeviceHandle* device, trace::VkSwapChain* swapchain)
	{
		vkDestroySwapchainKHR(device->m_device, swapchain->m_handle, instance->m_alloc_callback);
	}



}