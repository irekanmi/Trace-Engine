#pragma once

#include "core/Enums.h"
#include "core/io/Logging.h"
#include "vulkan/vulkan.h"


namespace trace {

#define VK_ASSERT(func) TRC_ASSERT(func == VK_SUCCESS, #func);

	struct VkHandle
	{
		VkInstance m_instance;
		VkAllocationCallbacks* m_alloc_callback = nullptr;
#ifdef TRC_DEBUG
		VkDebugUtilsMessengerEXT m_debugutils;
#endif
	};

}
