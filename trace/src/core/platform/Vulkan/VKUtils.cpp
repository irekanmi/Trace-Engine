#include "pch.h"

#include "VkUtils.h"
#include "EASTL/vector.h"
#include "core/Platform.h"

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

namespace vk {

	eastl::vector<const char*> g_validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkResult _CreateInstance(trace::VkHandle* instance)
	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.apiVersion = VK_API_VERSION_1_2;
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pApplicationName = "TestApp";
		app_info.pEngineName = _NAME_;
		app_info.pNext = nullptr;


		uint32_t ext_count = 0;
		const char** plat_ext = trace::Platform::GetExtensions(ext_count);

		eastl::vector<const char*> extensions;

		for (uint32_t i = 0; i < ext_count; i++)
		{
			extensions.push_back(plat_ext[i]);
		}

#ifdef TRC_DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledLayerCount = 0;
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
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
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

}