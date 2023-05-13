#include "pch.h"
#include "VKContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "VKtypes.h"
#include "EASTL/array.h"
#include "VkUtils.h"

trace::VKHandle g_Vkhandle;


namespace vk {

	bool __CreateContext(trace::GContext* context)
	{
		bool result = true;

		TRC_INFO("Added Vulkan Context Create Function :)");

		if (!context)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)context, __FUNCTION__);
			return false;
		}

		context->GetRenderHandle()->m_internalData = &g_Vkhandle;

		trace::VKHandle* _handle =  (trace::VKHandle*)context->GetRenderHandle()->m_internalData;

		VkResult res = vk::_CreateInstance(_handle);

		VK_ASSERT(res);

		TRC_INFO("Vulkan Instance Created");

#ifdef TRC_DEBUG_BUILD
		vk::EnableValidationlayers(_handle);
		TRC_INFO("Vulkan Debug utils Created");
#endif

		VK_ASSERT(vk::_CreateSurface(_handle));

		if (res != VK_SUCCESS)
		{
			context->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}

		return result;
	}

	bool __DestroyContext(trace::GContext* context)
	{
		bool result = true;

		TRC_INFO("Added Vulkan Context Destory Function :)");

		if (!context)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)context, __FUNCTION__);
			return false;
		}

		if (!context->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These context is invalid -> {}, Function -> {}", (const void*)context->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKHandle* _handle = (trace::VKHandle*)context->GetRenderHandle()->m_internalData;

		vk::_DestorySurface(_handle);

#ifdef TRC_DEBUG_BUILD
		vk::DisableValidationlayers(_handle);
#endif
		vk::_DestroyInstance(_handle);


		return result;
	}
	
}