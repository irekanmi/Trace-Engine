#include "pch.h"

#include "VkUtils.h"
#include "EASTL/vector.h"
#include "core/Application.h"
#include "vulkan/vulkan_win32.h"
#include "EASTL/map.h"
#include "VulkanShader.h"
#include "core/Platform.h"
#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_glsl.hpp"
#include "render/GRenderPass.h"

struct PhyScr
{
	VkPhysicalDevice phy_device;
	uint32_t score;
};



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
#ifdef TRC_DEBUG_BUILD
		"VK_LAYER_KHRONOS_validation"
#endif
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
				TRC_ERROR("Vaildation layer not found {}", i);
				TRC_ASSERT(false, "Vaildation layer not found {}", i);
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
				TRC_ERROR("Extension not found {}", i);
				TRC_ASSERT(false, "Extension not found {}", i);
				break;
			}

		}

		VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
#ifdef TRC_DEBUG_BUILD
		create_info.enabledLayerCount = static_cast<uint32_t>(g_validation_layers.size());
		create_info.ppEnabledLayerNames = g_validation_layers.data();
#endif
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
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
				TRC_ERROR("Vaildation layer not found {}", i);
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

		TRC_INFO("GPU Device Name: {}", phy_prop.deviceName);

		device->m_properties = phy_prop;
		device->m_features = phy_feat;

		uint32_t queue_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device->m_physicalDevice, &queue_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_props(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device->m_physicalDevice, &queue_count, queue_props.data());


		TRC_INFO(" Graphics | Compute | Transfer | Present ");
		bool used_queues[4] = { false };
		int n = 0;
		uint32_t index_count = 0;
		for (auto& i : queue_props)
		{

			if (i.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				device->m_queues.graphics_queue = n;
				if (!used_queues[n])
				{
					used_queues[n] = true;
					index_count++;
				}
				
			}

			if (i.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				device->m_queues.transfer_queue = n;
				if (!used_queues[n])
				{
					used_queues[n] = true;
					index_count++;
				}
			}

			if (i.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				device->m_queues.compute_queue = n;
				if (!used_queues[n])
				{
					used_queues[n] = true;
					index_count++;
				}
			}

			VkBool32 present_avaliable = 0;
			vkGetPhysicalDeviceSurfaceSupportKHR(device->m_physicalDevice, n, instance->m_surface, &present_avaliable);
			if (present_avaliable)
			{
				device->m_queues.present_queue = n;
				if (!used_queues[n])
				{
					used_queues[n] = true;
					index_count++;
				}
			}

			n++;
		}
		
		TRC_TRACE(" {} | {} | {} | {} ", device->m_queues.graphics_queue, device->m_queues.compute_queue, device->m_queues.transfer_queue, device->m_queues.present_queue);


				

		std::vector<uint32_t> indices(index_count);
		for (uint32_t i = 0; i < index_count; i++)
		{
			indices[i] = i;
		}

		


		// Initializing Bindless descriptors --------------------------------------------
		VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures depth_stencil_layout_features = {};
		depth_stencil_layout_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES;
		depth_stencil_layout_features.separateDepthStencilLayouts = VK_TRUE;
		depth_stencil_layout_features.pNext = nullptr;

		VkPhysicalDeviceDescriptorIndexingFeatures des_indexing_feat = {};
		des_indexing_feat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		des_indexing_feat.pNext = &depth_stencil_layout_features;

		VkPhysicalDeviceFeatures2 phy_feat2 = {};
		phy_feat2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		phy_feat2.pNext = &des_indexing_feat;

		// Fetch all features from physical device
		vkGetPhysicalDeviceFeatures2(device->m_physicalDevice, &phy_feat2);

		// Non-uniform indexing and update after bind
		TRC_ASSERT(des_indexing_feat.shaderSampledImageArrayNonUniformIndexing, "GPU doesn't support non-uniform indexing for sampled image");
		TRC_ASSERT(des_indexing_feat.descriptorBindingSampledImageUpdateAfterBind, "GPU doesn't support update after bind for sampled image");
		//TRC_ASSERT(des_indexing_feat.shaderUniformBufferArrayNonUniformIndexing, "GPU doesn't support non-uniform indexing for uniform buffer");
		//TRC_ASSERT(des_indexing_feat.descriptorBindingUniformBufferUpdateAfterBind, "GPU doesn't support update after bind for uniform buffer");
		//TRC_ASSERT(des_indexing_feat.shaderStorageBufferArrayNonUniformIndexing, "GPU doesn't support non-uniform indexing for storage buffer");
		//TRC_ASSERT(des_indexing_feat.descriptorBindingStorageBufferUpdateAfterBind, "GPU doesn't support update after bind for stroage buffer");
		TRC_ASSERT(des_indexing_feat.runtimeDescriptorArray, "GPU doesn't support runtimeDescriptorArray");
		TRC_ASSERT(des_indexing_feat.descriptorBindingPartiallyBound, "GPU doesn't support descriptorBindingPartiallyBound");

		// ------------------------------------------------------------------------------


		uint32_t present_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->m_physicalDevice, instance->m_surface, &present_count, nullptr);
		std::vector<VkPresentModeKHR> present_modes(present_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->m_physicalDevice, instance->m_surface, &present_count, present_modes.data());

		device->m_swapchainInfo.present_count = present_count;
		device->m_swapchainInfo.present_modes = present_modes;

		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_physicalDevice, instance->m_surface, &format_count, nullptr);
		std::vector<VkSurfaceFormatKHR> format_modes(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->m_physicalDevice, instance->m_surface, &format_count, format_modes.data());

		device->m_swapchainInfo.format_count = format_count;
		device->m_swapchainInfo.formats = format_modes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->m_physicalDevice, instance->m_surface, &device->m_swapchainInfo.surface_capabilities);


		std::vector<VkDeviceQueueCreateInfo> device_queue_infos(index_count);

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
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
		};

		//Disabled features ........
		phy_feat2.features.robustBufferAccess = VK_FALSE;

		//Required features ........
		phy_feat2.features.samplerAnisotropy = VK_TRUE;
		phy_feat2.features.geometryShader = VK_TRUE;
		phy_feat2.features.wideLines = VK_TRUE;

		VkDeviceCreateInfo device_info = {};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_infos.size());
		device_info.pQueueCreateInfos = device_queue_infos.data();
		device_info.pEnabledFeatures = nullptr;//&phy_feat;
		device_info.enabledLayerCount = 0;
		device_info.ppEnabledLayerNames = nullptr;
		device_info.enabledExtensionCount = 2;
		device_info.ppEnabledExtensionNames = device_extensions;
		device_info.pNext = &phy_feat2;

		VkResult device_created = vkCreateDevice(device->m_physicalDevice, &device_info, g_Vkhandle.m_alloc_callback, &device->m_device);
		TRC_INFO("Vulkan device creation result: {}", vulkan_result_string(device_created, true));

		TRC_TRACE("Obtaining Queues...");
		vkGetDeviceQueue(device->m_device, device->m_queues.graphics_queue, 0, &device->m_graphicsQueue);
		vkGetDeviceQueue(device->m_device, device->m_queues.present_queue, 0, &device->m_presentQueue);
		vkGetDeviceQueue(device->m_device, device->m_queues.transfer_queue, 0, &device->m_transferQueue);
		TRC_INFO("Queues Acquired");

		TRC_ASSERT(FindDepthFormat(device), "Depth Format not found");

		VkCommandPoolCreateInfo graphics_pool_info = {};
		graphics_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphics_pool_info.queueFamilyIndex = device->m_queues.graphics_queue;
		graphics_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_ASSERT(vkCreateCommandPool(device->m_device, &graphics_pool_info, instance->m_alloc_callback, &device->m_graphicsCommandPool));
		TRC_INFO("Graphics command pool created");

		//Per Frame descriptor buffer
		{
			for (uint32_t i = 0; i < VK_MAX_NUM_FRAMES; i++)
			{
				vk::createBuffer(
					instance,
					device,
					MB,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					device->m_frameDescriptorBuffer[i].m_handle,
					device->m_frameDescriptorBuffer[i].m_memory
				);

				vkMapMemory(
					device->m_device,
					device->m_frameDescriptorBuffer[i].m_memory,
					0,
					MB,
					VK_NO_FLAGS,
					&device->m_bufferPtr[i]
				);

				device->m_bufferData[i] = new char[MB];
			}
		}






		return device_created;
	}

	void _DestoryDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance)
	{

		//Per Frame descriptor buffer
		for (uint32_t i = 0; i < VK_MAX_NUM_FRAMES; i++)
		{
			delete[] device->m_bufferData[i];
			vkUnmapMemory(
				device->m_device,
				device->m_frameDescriptorBuffer[i].m_memory
			);

			_DestoryBuffer(instance, device, &device->m_frameDescriptorBuffer[i]);
		}

		// Graphics command pool
		vkDestroyCommandPool(device->m_device, device->m_graphicsCommandPool, instance->m_alloc_callback);

		vkDestroyDevice(device->m_device, instance->m_alloc_callback);
	}

	VkResult _CreateSurface(trace::VKHandle* instance)
	{
		VkResult res = VK_ERROR_INITIALIZATION_FAILED;

		switch (trace::AppSettings::platform_api)
		{
		case trace::PlatformAPI::WINDOWS:
		{

			VkWin32SurfaceCreateInfoKHR create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.hinstance = (HINSTANCE)trace::Platform::GetAppHandle();
			create_info.hwnd = (HWND)trace::Application::get_instance()->GetWindow()->GetNativeHandle();

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

		int index = 0;
		uint32_t score = 0;
		int j = 0;
		for (auto& i : device_board)
		{
			RatePhysicalDevice(i.phy_device, i.score);

			if (i.score > score)
			{
				index = j;
				score = i.score;
			}

			j++;
		}
		
		
		return device_board[index].phy_device;
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

		TRC_INFO("Device Name: {}; Score: {}", phy_prop.deviceName, score);

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
			if (i.format == VK_FORMAT_R8G8B8A8_UNORM && i.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surface_found = true;
				surface_format = i;
			}
			else if (i.format == VK_FORMAT_R16G16B16A16_SFLOAT && i.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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

		TRC_INFO("Minimum image count: {}", device->m_swapchainInfo.surface_capabilities.minImageCount);
		TRC_INFO("Maximum image count: {}", device->m_swapchainInfo.surface_capabilities.maxImageCount);
		TRC_INFO("Image count: {}", image_count);

		image_count = device->frames_in_flight;


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

		uint32_t queues[] = { device->m_queues.graphics_queue, device->m_queues.present_queue };
		if (device->m_queues.graphics_queue != device->m_queues.present_queue)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queues;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 1;
			create_info.pQueueFamilyIndices = &device->m_queues.graphics_queue;
		}

		VkResult swapchain_create_result = vkCreateSwapchainKHR(device->m_device, &create_info, instance->m_alloc_callback, &swapchain->m_handle);
		VK_ASSERT(swapchain_create_result);

		device->m_frameBufferWidth = width;
		device->m_frameBufferHeight = height;

		TRC_TRACE("Obtaining Vulkan swapchain images...");
		image_count = 0;
		vkGetSwapchainImagesKHR(device->m_device, swapchain->m_handle, &image_count, nullptr);
		std::vector<VkImage> swap_images(image_count);
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

		swapchain->m_depthFormat = device->m_depthFormat;

		/*_CreateImage(
			instance,
			device,
			&swapchain->m_depthimage,
			device->m_depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			0,
			VK_IMAGE_TYPE_2D,
			extent.width,
			extent.height,
			1,
			1
		);

		_CreateImageView(instance, device, &swapchain->m_depthimage.m_view, device->m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, &swapchain->m_depthimage.m_handle);*/

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
		else if (result == VK_SUBOPTIMAL_KHR)
		{
			_RecreateSwapchain(instance, device, swapchain, device->m_frameBufferWidth, device->m_frameBufferHeight);
			return false;
		}
		else if (result != VK_SUCCESS /*&& result != VK_SUBOPTIMAL_KHR*/)
		{
			TRC_CRITICAL(" Failed to acquire swapchain image, error message: {} ", vulkan_result_string(result, true));
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

		TRC_ASSERT(result == VK_SUCCESS, "Failed to present swapchain image");

		device->m_currentFrame = (device->m_currentFrame + 1) % device->frames_in_flight;

		return;
	}

	VkResult _CreateImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* out_image, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_prop, VkImageAspectFlags aspect_flags, VkImageCreateFlags flags, VkImageType image_type, uint32_t width, uint32_t height, uint32_t layers, uint32_t mip_levels)
	{
		VkImageCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.imageType = image_type;
		create_info.extent.width = width;
		create_info.extent.height = height;
		create_info.extent.depth = 1;
		create_info.format = format;
		create_info.tiling = tiling;
		create_info.arrayLayers = layers;	
		create_info.mipLevels = mip_levels;	
		create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.usage = usage;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.flags = flags;
		

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
			TRC_ASSERT(false, "Note that the device is not actually out memory but a vaild memory heap has not been found, So find a better error description");
			//TODO: Note that the device is not actually out memory but a vaild memory heap has not been found, So find a better error description
			return VK_ERROR_OUT_OF_DEVICE_MEMORY;
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
			vkDeviceWaitIdle(device->m_device);
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

	bool _TransitionImageLayout(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer,trace::VKImage* image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange sub_resource_range)
	{

		VkImageMemoryBarrier image_barrier = {};
		image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_barrier.image = image->m_handle;
		image_barrier.oldLayout = old_layout;
		image_barrier.newLayout = new_layout;
		image_barrier.srcQueueFamilyIndex = device->m_queues.graphics_queue;
		image_barrier.dstQueueFamilyIndex = device->m_queues.graphics_queue;

		image_barrier.subresourceRange = sub_resource_range;

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
		else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//TODO Check docs for more info
			src_stage_flag = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dst_stage_flag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			image_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			//TODO Check docs for more info
			src_stage_flag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dst_stage_flag = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_barrier.srcAccessMask = 0;
			image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			//TODO Check docs for more info
			src_stage_flag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
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

	bool _GenerateMipLevels(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* image, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layer_index)
	{
		bool result = true;

		VkFormatProperties format_prop;
		vkGetPhysicalDeviceFormatProperties(
			device->m_physicalDevice,
			format,
			&format_prop
		);

		if (!(format_prop.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			result = false;
			TRC_ERROR("The image format does not support image bliting (SAMPLED_IMAGE_FILTER_LINEAR_BIT) feature");
			return result;
		}

		trace::VKCommmandBuffer cmd_buf;
		_BeginCommandBufferSingleUse(
			device,
			device->m_graphicsCommandPool,
			&cmd_buf
		);

		VkImageMemoryBarrier mem_bar = {};
		mem_bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		mem_bar.image = image->m_handle;
		mem_bar.pNext = nullptr;
		mem_bar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		mem_bar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		mem_bar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mem_bar.subresourceRange.baseArrayLayer = layer_index;
		mem_bar.subresourceRange.layerCount = 1;
		mem_bar.subresourceRange.levelCount = 1;

		VkImageMemoryBarrier mem_bar1 = {};
		mem_bar1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		mem_bar1.image = image->m_handle;
		mem_bar1.pNext = nullptr;
		mem_bar1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		mem_bar1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		mem_bar1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		mem_bar1.subresourceRange.baseArrayLayer = layer_index;
		mem_bar1.subresourceRange.layerCount = 1;
		mem_bar1.subresourceRange.levelCount = 1;

		int32_t mip_width = static_cast<int32_t>(width);
		int32_t mip_height = static_cast<int32_t>(height);
		for (uint32_t i = 1; i < mip_levels; i++)
		{
			mem_bar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			mem_bar.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			mem_bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			mem_bar.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			mem_bar.subresourceRange.baseMipLevel = i - 1;

			mem_bar1.srcAccessMask = 0;
			mem_bar1.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			mem_bar1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			mem_bar1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			mem_bar1.subresourceRange.baseMipLevel = i;

			vkCmdPipelineBarrier(
				cmd_buf.m_handle,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&mem_bar
			);

			vkCmdPipelineBarrier(
				cmd_buf.m_handle,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&mem_bar1
			);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = {0, 0, 0};
			blit.srcOffsets[1] = {mip_width, mip_height, 1};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.baseArrayLayer = layer_index;
			blit.srcSubresource.layerCount = 1;
			blit.srcSubresource.mipLevel = i - 1;

			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.baseArrayLayer = layer_index;
			blit.dstSubresource.layerCount = 1;
			blit.dstSubresource.mipLevel = i;


			vkCmdBlitImage(
				cmd_buf.m_handle,
				image->m_handle,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image->m_handle,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_LINEAR // TODO: Get more info on image bliting to be able to make it more configurable
			);


			mem_bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			mem_bar.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			mem_bar.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			mem_bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				cmd_buf.m_handle,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &mem_bar
			);

			if (mip_width > 1) mip_width /= 2;
			if (mip_height > 1) mip_height /= 2;

		}

		mem_bar.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		mem_bar.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		mem_bar.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		mem_bar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		mem_bar.subresourceRange.baseMipLevel = mip_levels - 1;

		vkCmdPipelineBarrier(
			cmd_buf.m_handle,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &mem_bar
		);

		_EndCommandBufferSingleUse(
			device,
			device->m_graphicsCommandPool,
			device->m_graphicsQueue,
			&cmd_buf
		);



		return result;
	}

	VkResult _CreateSampler(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::TextureDesc& desc, VkSampler& sampler, float max_lod)
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
		create_info.maxLod = max_lod;
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

		return INVALID_ID;
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

		VkResult res = vkQueueWaitIdle(queue);
		if(res != VK_SUCCESS) TRC_INFO(vulkan_result_string(res, true));
		VK_ASSERT(res);

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

		//TODO: Find a way to provide number of avaliable swapchain images
		if (device->m_graphicsCommandBuffers.empty())
		{
			buffers.resize(3);

			for (uint32_t i = 0; i < 3; i++)
			{
				_AllocateCommandBuffer(device, command_pool, &buffers[i]);
			}
		}
		else
		{
			device->m_graphicsCommandBuffers.clear();
			buffers.resize(3);


			for (uint32_t i = 0; i < 3; i++)
			{
				_AllocateCommandBuffer(device, command_pool, &buffers[i]);
			}
		}

	}

	VkResult _CreateFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer,const eastl::vector<VkImageView>& attachments, trace::VKRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t attachment_count)
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
		TRC_ERROR("Can't destory an invaild fence line: {} \n ==>Function: {}", __LINE__, __FUNCTION__);
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
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

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
		render_pass->clear_color = &clear_color;
		render_pass->depth_value = &depth_value;
		render_pass->stencil_value = &stencil_value;
		render_pass->render_area = &render_area;


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
		begin_info.renderArea.offset.x = static_cast<int32_t>(render_pass->render_area->x);
		begin_info.renderArea.offset.y = static_cast<int32_t>(render_pass->render_area->y);
		begin_info.renderArea.extent.width = static_cast<uint32_t>(render_pass->render_area->z);
		begin_info.renderArea.extent.height = static_cast<uint32_t>(render_pass->render_area->w);
		begin_info.framebuffer = framebuffer;
		
		VkClearValue clear_colors[2] = {};
		clear_colors[0].color.float32[0] = render_pass->clear_color->r;
		clear_colors[0].color.float32[1] = render_pass->clear_color->g;
		clear_colors[0].color.float32[2] = render_pass->clear_color->b;
		clear_colors[0].color.float32[3] = render_pass->clear_color->a;

		clear_colors[1].depthStencil.depth = *render_pass->depth_value;
		clear_colors[1].depthStencil.stencil = *render_pass->stencil_value;

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

	VkResult _CreatePipeline(trace::VKHandle* instance, trace::VKDeviceHandle* device, uint32_t view_port_count, VkViewport* view_ports, uint32_t scissor_count, VkRect2D* scissors, trace::PipelineStateDesc desc, trace::VKPipeline* out_pipeline, trace::VKRenderPass* render_pass, uint32_t subpass_index)
	{
		VkResult result = VK_ERROR_INITIALIZATION_FAILED;

		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

		if (desc.vertex_shader != nullptr)
		{
			trace::VKShader* shader = reinterpret_cast<trace::VKShader*>(desc.vertex_shader->GetRenderHandle()->m_internalData);
			shader_stages.push_back(shader->create_info);
		}
		if (desc.pixel_shader != nullptr)
		{
			trace::VKShader* shader = reinterpret_cast<trace::VKShader*>(desc.pixel_shader->GetRenderHandle()->m_internalData);
			shader_stages.push_back(shader->create_info);
		}

		VkVertexInputBindingDescription binding;
		eastl::vector<VkVertexInputAttributeDescription> attrs;

		parseInputLayout(desc.input_layout, binding, attrs);
	
		VkPipelineVertexInputStateCreateInfo vert_info = {};
		vert_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vert_info.vertexBindingDescriptionCount = !attrs.empty() ?  1 : 0; // TODO
		vert_info.pVertexBindingDescriptions = &binding;
		vert_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrs.size());
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
		parseRasterizerState( desc.rasteriser_state,raster_info);

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
		VkPipelineColorBlendAttachmentState attach_states[10] = {};


		color_blend_info.attachmentCount = desc.render_pass->GetColorAttachmentCount();
		for (uint32_t i = 0; i < color_blend_info.attachmentCount; i++)
		{
			attach_states[i] = attach_state;
		}
		color_blend_info.pAttachments = attach_states;

		
		VkPipelineLayoutCreateInfo layout_info = {};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		VkDescriptorSetLayout _layouts[3];
		std::vector<VkPushConstantRange> ranges;
		parsePipelineLayout(instance, device, desc, layout_info, out_pipeline, _layouts, ranges);
		
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
		create_info.renderPass = render_pass->m_handle; // TODO: Maybe can be dynamic
		create_info.basePipelineHandle = VK_NULL_HANDLE; // TODO:  Check Docs
		create_info.basePipelineIndex = -1; // TODO
		create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
		create_info.pStages = shader_stages.data();
		create_info.subpass = subpass_index; // TODO
		create_info.layout = out_pipeline->m_layout;

		result = vkCreateGraphicsPipelines(
			device->m_device,
			VK_NULL_HANDLE, // TODO: Check Docs for more info
			1,
			&create_info,
			instance->m_alloc_callback,
			&out_pipeline->m_handle
			);

		TRC_INFO(vulkan_result_string(result, true));

		VK_ASSERT(result);

		return result;
	}

	void _DestroyPipeline(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKPipeline* pipeline)
	{

		
		if (pipeline->Scene_layout)
		{
			vkDestroyDescriptorSetLayout(device->m_device, pipeline->Scene_layout, instance->m_alloc_callback);
			pipeline->Scene_layout = VK_NULL_HANDLE;
			if (pipeline->Scene_pool)
			{
				vkDestroyDescriptorPool(device->m_device, pipeline->Scene_pool, instance->m_alloc_callback);
				pipeline->Scene_pool = VK_NULL_HANDLE;
			}
		}
		
		if (pipeline->Instance_layout)
		{
			vkDestroyDescriptorSetLayout(device->m_device, pipeline->Instance_layout, instance->m_alloc_callback);
			pipeline->Instance_layout = VK_NULL_HANDLE;
			if (pipeline->Instance_pool)
			{
				vkDestroyDescriptorPool(device->m_device, pipeline->Instance_pool, instance->m_alloc_callback);
				pipeline->Instance_pool = VK_NULL_HANDLE;
			}
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
		

		//out_shader->m_code = code;

		//VkShaderModuleCreateInfo create_info = {};
		//create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		//create_info.codeSize = out_shader->m_code.size() * sizeof(uint32_t);
		//create_info.pCode = out_shader->m_code.data();

		//result = vkCreateShaderModule(device->m_device, &create_info, instance->m_alloc_callback, &out_shader->m_module);


		//VK_ASSERT(result);

		//out_shader->create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		//out_shader->create_info.module = out_shader->m_module;
		//out_shader->create_info.pName = "main";
		//out_shader->create_info.stage = convertShaderStage(stage);
		//out_shader->create_info.pSpecializationInfo = nullptr; // TODO: Check Docs for more info


		return VkResult();
	}

	void _DestoryShader(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKShader* shader)
	{
		if (shader->m_module)
		{
			vkDeviceWaitIdle(device->m_device);
			vkDestroyShaderModule(device->m_device, shader->m_module, instance->m_alloc_callback);
		}

	}

	VkResult _CreateBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* out_buffer, trace::BufferInfo buffer_info)
	{
		VkResult result;

		//auto usage = convertBufferUsage(buffer_info.m_usage);

		VkBufferCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: Check Docs for more info
		create_info.size = buffer_info.m_size;

		VkBufferUsageFlags usage_flag = 0;
		VkMemoryPropertyFlags memory_property = 0;

		if (TRC_HAS_FLAG(buffer_info.m_flag, trace::BindFlag::VERTEX_BIT))
		{
			usage_flag |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (TRC_HAS_FLAG(buffer_info.m_flag, trace::BindFlag::INDEX_BIT))
		{
			usage_flag |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (TRC_HAS_FLAG(buffer_info.m_flag, trace::BindFlag::CONSTANT_BUFFER_BIT))
		{
			usage_flag |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (TRC_HAS_FLAG(buffer_info.m_flag, trace::BindFlag::UNORDERED_RESOURCE_BIT))
		{
			usage_flag |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}
		if (buffer_info.m_usageFlag == trace::UsageFlag::UPLOAD)
		{
			memory_property |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			usage_flag |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		if (buffer_info.m_usageFlag == trace::UsageFlag::DEFAULT)
		{
			memory_property |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		create_info.usage = usage_flag;


		result = vkCreateBuffer(device->m_device, &create_info, instance->m_alloc_callback, &out_buffer->m_handle);

		VK_ASSERT(result);
		out_buffer->m_info = buffer_info;

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(device->m_device, out_buffer->m_handle, &mem_requirements);

		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = FindMemoryIndex(device, mem_requirements.memoryTypeBits, memory_property);

		VK_ASSERT(vkAllocateMemory(device->m_device, &alloc_info, instance->m_alloc_callback, &out_buffer->m_memory));

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

	void _ResizeBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer& buffer, uint32_t new_size)
	{
		trace::BufferInfo new_info = buffer.m_info;
		if (new_size < new_info.m_size)
		{
			TRC_WARN("Can't resize a buffer to a size lower than it's current size: {}", __FUNCTION__);
			return;
		}

		trace::VKBuffer new_buffer;
		new_info.m_size = new_size;
		_CreateBuffer(instance, device, &new_buffer, new_info);

		_CopyBuffer(instance, device, &buffer, &new_buffer, buffer.m_info.m_size, 0, 0);

		device->frames_resources[device->m_imageIndex]._buffers.push_back(buffer.m_handle);
		device->frames_resources[device->m_imageIndex]._memorys.push_back(buffer.m_memory);
		
		buffer = new_buffer;
	}

	void _ResizeBufferQueue(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer& buffer, uint32_t new_size)
	{
		trace::BufferInfo new_info = buffer.m_info;
		if (new_size < new_info.m_size)
		{
			TRC_WARN("Can't resize a buffer to a size lower than it's current size: {}", __FUNCTION__);
			return;
		}

		trace::VKBuffer new_buffer;
		new_info.m_size = new_size;
		_CreateBuffer(instance, device, &new_buffer, new_info);

		_CopyBufferQueue(instance, device, &buffer, &new_buffer, buffer.m_info.m_size, 0, 0);

		buffer = new_buffer;
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

	void _CopyBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* src, trace::VKBuffer* dst, uint32_t size, uint32_t src_offset, uint32_t dst_offset)
	{
		trace::VKCommmandBuffer cmd_buf;
		_BeginCommandBufferSingleUse(device, device->m_graphicsCommandPool, &cmd_buf);

		VkBufferCopy copy_region = {};
		copy_region.size = size;
		copy_region.dstOffset = dst_offset;
		copy_region.srcOffset = src_offset;

		vkCmdCopyBuffer(cmd_buf.m_handle, src->m_handle, dst->m_handle, 1, &copy_region);

		_EndCommandBufferSingleUse(device, device->m_graphicsCommandPool, device->m_graphicsQueue, &cmd_buf);




	}

	void _CopyBufferQueue(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* src, trace::VKBuffer* dst, uint32_t size, uint32_t src_offset, uint32_t dst_offset)
	{
		trace::VKCommmandBuffer cmd_buf = device->m_graphicsCommandBuffers[device->m_imageIndex];

		VkBufferCopy copy_region = {};
		copy_region.size = size;
		copy_region.dstOffset = dst_offset;
		copy_region.srcOffset = src_offset;

		vkCmdCopyBuffer(cmd_buf.m_handle, src->m_handle, dst->m_handle, 1, &copy_region);



	}

	void parseInputLayout(trace::InputLayout& layout, VkVertexInputBindingDescription& binding, eastl::vector<VkVertexInputAttributeDescription>& attrs)
	{

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

	void parseRasterizerState(trace::RasterizerState& raterizer, VkPipelineRasterizationStateCreateInfo& create_info)
	{

		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		create_info.lineWidth = 1.0f; // TODO
		create_info.polygonMode = convertPolygonMode(raterizer.fill_mode);
		create_info.cullMode = convertCullMode(raterizer.cull_mode);
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

		if (state.alpha_to_blend_coverage)
		{
			// ------------------ TODO----------------------------------------
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = convertBlendFactor(state.src_color); //
			colorBlendAttachment.dstColorBlendFactor = convertBlendFactor(state.dst_color); //
			colorBlendAttachment.colorBlendOp = convertBlendOp(state.color_op); // Optional
			colorBlendAttachment.srcAlphaBlendFactor = convertBlendFactor(state.src_alpha); //
			colorBlendAttachment.dstAlphaBlendFactor = convertBlendFactor(state.dst_alpha); //
			colorBlendAttachment.alphaBlendOp = convertBlendOp(state.alpha_op); // Optional
			//------------------------------------------------------------------
		}
		
		


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

	void parsePipelineLayout(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::PipelineStateDesc& desc, VkPipelineLayoutCreateInfo& create_info, trace::VKPipeline* pipeline, VkDescriptorSetLayout* _layouts, std::vector<VkPushConstantRange>& ranges)
	{


		VkResult result;


		std::vector<VkDescriptorSetLayoutBinding> SceneGlobalData_bindings;

		std::vector<VkDescriptorSetLayoutBinding> Instance_bindings;


		const uint32_t pool_sizes_count = 3;
		VkDescriptorPoolSize pool_sizes[] =
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, KB * VK_MAX_NUM_FRAMES},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (KB * 12)}
		};


		
		uint32_t bindings_count = 0;
		for (auto& i : desc.resources.resources)
		{
			bool is_structure = i.def == trace::ShaderDataDef::STRUCTURE;
			bool is_image = i.def == trace::ShaderDataDef::IMAGE;

			trace::ShaderResourceStage res_stage = i.resource_stage;


			switch (res_stage)
			{
			case trace::ShaderResourceStage::RESOURCE_STAGE_GLOBAL:
			{
				VkDescriptorSetLayoutBinding bind = {};
				bind.binding = i.slot;
				bind.descriptorCount = i.count;
				bind.descriptorType = convertDescriptorType(i.resource_type);
				bind.stageFlags = convertShaderStage(i.shader_stage);

				SceneGlobalData_bindings.push_back(bind);

				break;
			}
			case trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE:
			{
				VkDescriptorSetLayoutBinding bind = {};
				bind.binding = i.slot;
				bind.descriptorCount = i.count;
				bind.descriptorType = convertDescriptorType(i.resource_type);
				bind.stageFlags = convertShaderStage(i.shader_stage);
				bind.descriptorCount = is_structure ? KB / 2 : KB;
				Instance_bindings.push_back(bind);
				bindings_count++;
				break;
			}
			case trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL:
			{
				if (is_structure)
				{
					uint32_t offset = 0;
					VkPushConstantRange rag = {};
					for (auto& mem : i.members)
					{
						rag.offset = offset;
						rag.size = mem.resource_size;
						rag.stageFlags = convertShaderStage(i.shader_stage);
						ranges.push_back(rag);
						offset += mem.resource_size;
						offset = get_alignment(offset, 16);
					}
				}
				break;
			}

			}

			

		}
		uint32_t set_layout_count = 0;
		if (!desc.resources.resources.empty())
		{
			std::vector<VkDescriptorBindingFlags> binds_flags(bindings_count, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
			VkDescriptorSetLayoutBindingFlagsCreateInfo binds_flag = {};
			binds_flag.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			binds_flag.pNext = nullptr;
			binds_flag.bindingCount = bindings_count;
			binds_flag.pBindingFlags = binds_flags.data();


			if (!SceneGlobalData_bindings.empty())
			{
				VkDescriptorSetLayoutCreateInfo _info = {};
				_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				_info.bindingCount = static_cast<uint32_t>(SceneGlobalData_bindings.size());
				_info.pBindings = SceneGlobalData_bindings.data();
				//_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

				result = vkCreateDescriptorSetLayout(device->m_device, &_info, instance->m_alloc_callback, &pipeline->Scene_layout);

				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.maxSets = 8096;//1000 * pool_sizes_count;
				pool_info.poolSizeCount = pool_sizes_count;
				pool_info.pPoolSizes = pool_sizes;

				result = vkCreateDescriptorPool(device->m_device, &pool_info, instance->m_alloc_callback, &pipeline->Scene_pool);

				VkDescriptorSetLayout test[VK_MAX_NUM_FRAMES];
				for (uint32_t i = 0; i < VK_MAX_NUM_FRAMES; i++)
				{
					test[i] = pipeline->Scene_layout;
				}

				VkDescriptorSetAllocateInfo alloc_info = {};
				alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				alloc_info.descriptorPool = pipeline->Scene_pool;
				alloc_info.descriptorSetCount = VK_MAX_NUM_FRAMES;
				alloc_info.pSetLayouts = test;


				result = vkAllocateDescriptorSets(device->m_device, &alloc_info, pipeline->Scene_sets);
				VK_ASSERT(result);
				_layouts[set_layout_count++] = pipeline->Scene_layout;

				//===============================================================
			}
			if (!Instance_bindings.empty())
			{
				pipeline->bindless = true;
				VkDescriptorSetLayoutCreateInfo _info = {};
				_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				_info.bindingCount = static_cast<uint32_t>(Instance_bindings.size());
				_info.pBindings = Instance_bindings.data();
				_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
				_info.pNext = &binds_flag;

				result = vkCreateDescriptorSetLayout(device->m_device, &_info, instance->m_alloc_callback, &pipeline->Instance_layout);

				VkDescriptorPoolCreateInfo pool_info = {};
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.maxSets = 1000 * pool_sizes_count;
				pool_info.poolSizeCount = pool_sizes_count;
				pool_info.pPoolSizes = pool_sizes;
				pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

				result = vkCreateDescriptorPool(device->m_device, &pool_info, instance->m_alloc_callback, &pipeline->Instance_pool);

				VkDescriptorSetLayout test[VK_MAX_NUM_FRAMES];
				for (uint32_t i = 0; i < VK_MAX_NUM_FRAMES; i++)
				{
					test[i] = pipeline->Instance_layout;
				}

				VkDescriptorSetAllocateInfo alloc_info = {};
				alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				alloc_info.descriptorPool = pipeline->Instance_pool;
				alloc_info.descriptorSetCount = VK_MAX_NUM_FRAMES;
				alloc_info.pSetLayouts = test;


				result = vkAllocateDescriptorSets(device->m_device, &alloc_info, pipeline->Instance_sets);

				TRC_TRACE(vulkan_result_string(result, true));
				VK_ASSERT(result);

				_layouts[set_layout_count++] = pipeline->Instance_layout;

				//===============================================================
			}

		}


		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = set_layout_count;
		create_info.pSetLayouts = _layouts;
		create_info.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
		create_info.pPushConstantRanges = ranges.data();

	}

	VkFormat convertFmt(trace::Format format)
	{
		VkFormat result = VK_FORMAT_UNDEFINED;

		switch (format)
		{
		case trace::Format::R32G32B32_FLOAT:
		{
			result = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		}
		case trace::Format::R32G32B32A32_FLOAT:
		{
			result = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		}
		case trace::Format::R32G32B32A32_UINT:
		{
			result = VK_FORMAT_R32G32B32A32_UINT;
			break;
		}
		case trace::Format::R32G32B32A32_SINT:
		{
			result = VK_FORMAT_R32G32B32A32_SINT;
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
		case trace::Format::R32_FLOAT:
		{
			result = VK_FORMAT_R32_SFLOAT;
			break;
		}
		case trace::Format::R32G32_UINT:
		{
			result = VK_FORMAT_R32G32_UINT;
			break;
		}
		case trace::Format::R32_UINT:
		{
			result = VK_FORMAT_R32_UINT;
			break;
		}
		case trace::Format::R32_SINT:
		{
			result = VK_FORMAT_R32_SINT;
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
		case trace::Format::R8G8B8A8_RBG:
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
		case trace::Format::R8G8B8A8_UNORM:
		{
			result = VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}
		case trace::Format::R8G8B8_UNORM:
		{
			result = VK_FORMAT_R8G8B8_UNORM;
			break;
		}
		case trace::Format::D32_SFLOAT_S8_SUINT:
		{
			result = VK_FORMAT_D32_SFLOAT_S8_UINT;
			break;
		}
		case trace::Format::D32_SFLOAT:
		{
			result = VK_FORMAT_D32_SFLOAT;
			break;
		}
		case trace::Format::R16G16B16A16_FLOAT:
		{
			result = VK_FORMAT_R16G16B16A16_SFLOAT;
			break;
		}
		case trace::Format::R16G16B16_FLOAT:
		{
			result = VK_FORMAT_R16G16B16_SFLOAT;
			break;
		}
		case trace::Format::R16_FLOAT:
		{
			result = VK_FORMAT_R16_SFLOAT;
			break;
		}
		case trace::Format::R8_UNORM:
		{
			result = VK_FORMAT_R8_UNORM;
			break;
		}
		}

		return result;
	}

	VkImageViewType convertImageViewType(trace::ImageType image_type)
	{

		switch (image_type)
		{
		case trace::ImageType::IMAGE_1D:
		{
			return VK_IMAGE_VIEW_TYPE_1D;
		}
		case trace::ImageType::IMAGE_2D:
		{
			return VK_IMAGE_VIEW_TYPE_2D;
		}
		case trace::ImageType::IMAGE_3D:
		{
			return VK_IMAGE_VIEW_TYPE_3D;
		}
		case trace::ImageType::CUBE_MAP:
		{
			return VK_IMAGE_VIEW_TYPE_CUBE;
		}
		}

		return VK_IMAGE_VIEW_TYPE_2D;
	}

	VkImageType convertImageType(trace::ImageType image_type)
	{
		switch (image_type)
		{
		case trace::ImageType::IMAGE_1D:
		{
			return VK_IMAGE_TYPE_1D;
		}
		case trace::ImageType::IMAGE_2D:
		{
			return VK_IMAGE_TYPE_2D;
		}
		case trace::ImageType::IMAGE_3D:
		{
			return VK_IMAGE_TYPE_3D;
		}
		case trace::ImageType::CUBE_MAP:
		{
			return VK_IMAGE_TYPE_2D;
		}
		}

		return VK_IMAGE_TYPE_2D;
	}

	VkPrimitiveTopology convertTopology(trace::PRIMITIVETOPOLOGY topology)
	{
		VkPrimitiveTopology result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		switch (topology)
		{
		case trace::PRIMITIVETOPOLOGY::TRIANGLE_LIST:
		{
			result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		}
		case trace::PRIMITIVETOPOLOGY::LINE_LIST:
		{
			result = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		}
		case trace::PRIMITIVETOPOLOGY::TRIANGLE_STRIP:
		{
			result = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		}
		case trace::PRIMITIVETOPOLOGY::LINE_STRIP:
		{
			result = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		}
		}

		return result;
	}

	VkPolygonMode convertPolygonMode(trace::FillMode fillmode)
	{

		switch (fillmode)
		{
		case trace::FillMode::SOLID:
		{
			return VK_POLYGON_MODE_FILL;
		}
		case trace::FillMode::WIREFRAME:
		{
			return VK_POLYGON_MODE_LINE;
		}
		}

		return VK_POLYGON_MODE_FILL;
	}

	VkShaderStageFlagBits convertShaderStage(trace::ShaderStage stage)
	{
		VkShaderStageFlagBits result = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		uint32_t val = 0;

		if (TRC_HAS_FLAG(stage, trace::ShaderStage::VERTEX_SHADER))
		{
			val |= VK_SHADER_STAGE_VERTEX_BIT;
		}
		if (TRC_HAS_FLAG(stage, trace::ShaderStage::PIXEL_SHADER))
		{
			val |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}


		result = (VkShaderStageFlagBits)val;

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
		case trace::AddressMode::CLAMP_TO_EDGE:
		{
			mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		}
		case trace::AddressMode::CLAMP_TO_BORDER:
		{
			mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
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
		case trace::FilterMode::NEAREST:
			{
				result = VK_FILTER_NEAREST;
				break;
			}
		}

		return result;
	}

	VkImageLayout convertTextureFmt(trace::TextureFormat fmt)
	{

		switch (fmt)
		{
		case trace::TextureFormat::COLOR_ATTACHMENT:
		{

			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		}
		case trace::TextureFormat::DEPTH_STENCIL:
		{

			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		}
		case trace::TextureFormat::DEPTH:
		{

			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			break;
		}
		case trace::TextureFormat::PRESENT:
		{

			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		}
		case trace::TextureFormat::SHADER_READ:
		{

			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		}
		case trace::TextureFormat::UNKNOWN:
		{

			return VK_IMAGE_LAYOUT_UNDEFINED;
			break;
		}
		}

		return VK_IMAGE_LAYOUT_MAX_ENUM;
	}

	VkAttachmentLoadOp convertAttachmentLoadOp(trace::AttachmentLoadOp op)
	{

		switch (op)
		{
		case trace::AttachmentLoadOp::LOAD_OP_CLEAR:
		{
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
			break;
		}
		case trace::AttachmentLoadOp::LOAD_OP_DISCARD:
		{
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			break;
		}
		case trace::AttachmentLoadOp::LOAD_OP_LOAD:
		{
			return VK_ATTACHMENT_LOAD_OP_LOAD;
			break;
		}

		}

		return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}

	VkAttachmentStoreOp convertAttachmentStoreOp(trace::AttachmentStoreOp op)
	{

		switch (op)
		{
		case trace::AttachmentStoreOp::STORE_OP_STORE:
		{
			return VK_ATTACHMENT_STORE_OP_STORE;
			break;
		}
		case trace::AttachmentStoreOp::STORE_OP_DISCARD:
		{
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		}
		}

		return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}

	VkCullModeFlagBits convertCullMode(trace::CullMode mode)
	{
		
		switch (mode)
		{
		case trace::CullMode::NONE:
		{
			return VK_CULL_MODE_NONE;
		}
		case trace::CullMode::FRONT:
		{
			return VK_CULL_MODE_FRONT_BIT;
		}
		case trace::CullMode::BACK:
		{
			return VK_CULL_MODE_BACK_BIT;
		}
		}

		return VK_CULL_MODE_NONE;
	}

	VkBlendFactor convertBlendFactor(trace::BlendFactor factor)
	{
		switch (factor)
		{
		case trace::BlendFactor::BLEND_ONE:
			{
				return VK_BLEND_FACTOR_ONE;
			}
		case trace::BlendFactor::BLEND_ZERO:
			{
				return VK_BLEND_FACTOR_ZERO;
			}
		case trace::BlendFactor::BLEND_ONE_MINUS_SRC_ALPHA:
			{
				return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			}
		case trace::BlendFactor::BLEND_ONE_MINUS_DST_ALPHA:
			{
				return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			}
		case trace::BlendFactor::BLEND_ONE_MINUS_SRC_COLOR:
			{
				return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			}
		case trace::BlendFactor::BLEND_ONE_MINUS_DST_COLOR:
			{
				return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			}
		case trace::BlendFactor::BLEND_SRC_COLOR:
			{
				return VK_BLEND_FACTOR_SRC_COLOR;
			}
		case trace::BlendFactor::BLEND_DST_COLOR:
			{
				return VK_BLEND_FACTOR_DST_COLOR;
			}
		case trace::BlendFactor::BLEND_SRC_ALPHA:
			{
				return VK_BLEND_FACTOR_SRC_ALPHA;
			}
		case trace::BlendFactor::BLEND_DST_ALPHA:
			{
			return VK_BLEND_FACTOR_DST_ALPHA;
			}
		}

		return VK_BLEND_FACTOR_MAX_ENUM;
	}

	VkBlendOp convertBlendOp(trace::BlendOp op)
	{
		switch (op)
		{
		case trace::BlendOp::BLEND_OP_ADD:
		{
		return VK_BLEND_OP_ADD;
		}
		case trace::BlendOp::BLEND_OP_SUBTRACT:
		{
		return VK_BLEND_OP_SUBTRACT;
		}
		case trace::BlendOp::BLEND_OP_REVERSE_SUBTRACT:
		{
		return VK_BLEND_OP_REVERSE_SUBTRACT;
		}
		case trace::BlendOp::BLEND_OP_MIN:
		{
		return VK_BLEND_OP_MIN;
		}
		case trace::BlendOp::BLEND_OP_MAX:
		{
		return VK_BLEND_OP_MAX;
		}
		}

		return VK_BLEND_OP_MAX_ENUM;
	}

	VkDescriptorType convertDescriptorType(trace::ShaderResourceType type)
	{
		switch (type)
		{
		case trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER:
		{
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
		case trace::ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
		{
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}
		case trace::ShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER:
		{
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}
		}

		return VkDescriptorType();
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

	bool operator==(VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs)
	{
		bool result = (lhs.binding == rhs.binding) &&
			(lhs.descriptorCount == rhs.descriptorCount) &&
			(lhs.descriptorType == rhs.descriptorType) &&
			(lhs.stageFlags == rhs.stageFlags);
		return result;
	}

	const char* vulkan_result_string(VkResult result, bool get_extended) {
		// From: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkResult.html
		// Success Codes
		switch (result) {
		default:
		case VK_SUCCESS:
			return !get_extended ? "VK_SUCCESS" : "VK_SUCCESS Command successfully completed";
		case VK_NOT_READY:
			return !get_extended ? "VK_NOT_READY" : "VK_NOT_READY A fence or query has not yet completed";
		case VK_TIMEOUT:
			return !get_extended ? "VK_TIMEOUT" : "VK_TIMEOUT A wait operation has not completed in the specified time";
		case VK_EVENT_SET:
			return !get_extended ? "VK_EVENT_SET" : "VK_EVENT_SET An event is signaled";
		case VK_EVENT_RESET:
			return !get_extended ? "VK_EVENT_RESET" : "VK_EVENT_RESET An event is unsignaled";
		case VK_INCOMPLETE:
			return !get_extended ? "VK_INCOMPLETE" : "VK_INCOMPLETE A return array was too small for the result";
		case VK_SUBOPTIMAL_KHR:
			return !get_extended ? "VK_SUBOPTIMAL_KHR" : "VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";
		case VK_THREAD_IDLE_KHR:
			return !get_extended ? "VK_THREAD_IDLE_KHR" : "VK_THREAD_IDLE_KHR A deferred operation is not complete but there is currently no work for this thread to do at the time of this call.";
		case VK_THREAD_DONE_KHR:
			return !get_extended ? "VK_THREAD_DONE_KHR" : "VK_THREAD_DONE_KHR A deferred operation is not complete but there is no work remaining to assign to additional threads.";
		case VK_OPERATION_DEFERRED_KHR:
			return !get_extended ? "VK_OPERATION_DEFERRED_KHR" : "VK_OPERATION_DEFERRED_KHR A deferred operation was requested and at least some of the work was deferred.";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return !get_extended ? "VK_OPERATION_NOT_DEFERRED_KHR" : "VK_OPERATION_NOT_DEFERRED_KHR A deferred operation was requested and no operations were deferred.";
		case VK_PIPELINE_COMPILE_REQUIRED_EXT:
			return !get_extended ? "VK_PIPELINE_COMPILE_REQUIRED_EXT" : "VK_PIPELINE_COMPILE_REQUIRED_EXT A requested pipeline creation would have required compilation, but the application requested compilation to not be performed.";

			// Error codes
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return !get_extended ? "VK_ERROR_OUT_OF_HOST_MEMORY" : "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return !get_extended ? "VK_ERROR_OUT_OF_DEVICE_MEMORY" : "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has failed.";
		case VK_ERROR_INITIALIZATION_FAILED:
			return !get_extended ? "VK_ERROR_INITIALIZATION_FAILED" : "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for implementation-specific reasons.";
		case VK_ERROR_DEVICE_LOST:
			return !get_extended ? "VK_ERROR_DEVICE_LOST" : "VK_ERROR_DEVICE_LOST The logical or physical device has been lost. See Lost Device";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return !get_extended ? "VK_ERROR_MEMORY_MAP_FAILED" : "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return !get_extended ? "VK_ERROR_LAYER_NOT_PRESENT" : "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or could not be loaded.";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return !get_extended ? "VK_ERROR_EXTENSION_NOT_PRESENT" : "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not supported.";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return !get_extended ? "VK_ERROR_FEATURE_NOT_PRESENT" : "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return !get_extended ? "VK_ERROR_INCOMPATIBLE_DRIVER" : "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return !get_extended ? "VK_ERROR_TOO_MANY_OBJECTS" : "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created.";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return !get_extended ? "VK_ERROR_FORMAT_NOT_SUPPORTED" : "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported on this device.";
		case VK_ERROR_FRAGMENTED_POOL:
			return !get_extended ? "VK_ERROR_FRAGMENTED_POOL" : "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to fragmentation of the pool�s memory. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool allocation failure was due to fragmentation.";
		case VK_ERROR_SURFACE_LOST_KHR:
			return !get_extended ? "VK_ERROR_SURFACE_LOST_KHR" : "VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return !get_extended ? "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" : "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again.";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return !get_extended ? "VK_ERROR_OUT_OF_DATE_KHR" : "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return !get_extended ? "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" : "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
		case VK_ERROR_INVALID_SHADER_NV:
			return !get_extended ? "VK_ERROR_INVALID_SHADER_NV" : "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile or link. More details are reported back to the application via VK_EXT_debug_report if enabled.";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return !get_extended ? "VK_ERROR_OUT_OF_POOL_MEMORY" : "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. If the failure was definitely due to fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return !get_extended ? "VK_ERROR_INVALID_EXTERNAL_HANDLE" : "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid handle of the specified type.";
		case VK_ERROR_FRAGMENTATION:
			return !get_extended ? "VK_ERROR_FRAGMENTATION" : "VK_ERROR_FRAGMENTATION A descriptor pool creation has failed due to fragmentation.";
		case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
			return !get_extended ? "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT" : "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed because the requested address is not available.";
			// NOTE: Same as above
			//case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			//    return !get_extended ? "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" :"VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS A buffer creation or memory allocation failed because the requested address is not available. A shader group handle assignment failed because the requested shader group handle information is no longer valid.";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return !get_extended ? "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" : "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT An operation on a swapchain created with VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have exlusive full-screen access. This may occur due to implementation-dependent reasons, outside of the application�s control.";
		case VK_ERROR_UNKNOWN:
			return !get_extended ? "VK_ERROR_UNKNOWN" : "VK_ERROR_UNKNOWN An unknown error has occurred; either the application has provided invalid input, or an implementation failure has occurred.";
		}
	}

}

