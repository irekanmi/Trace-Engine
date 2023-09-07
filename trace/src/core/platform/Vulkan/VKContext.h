#pragma once

#include "render/GContext.h"
#include "vulkan/vulkan.h"
#include "VKtypes.h"


namespace vk {

	bool __CreateContext(trace::GContext* context);
	bool __DestroyContext(trace::GContext* context);
	

}

