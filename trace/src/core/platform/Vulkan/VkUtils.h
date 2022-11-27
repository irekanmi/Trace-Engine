#pragma once
#include "vulkan/vulkan.h"
#include "VKtypes.h"
#include "EASTL/vector.h"

namespace vk {

	VkResult _CreateInstance(trace::VkHandle* instance);
	void _DestroyInstance(trace::VkHandle* instance);
	bool _FindLayer(const char* layer, eastl::vector<VkLayerProperties>& layers);
	void EnableValidationlayers(trace::VkHandle* instance);
	void DisableValidationlayers(trace::VkHandle* instance);

}
