#include "pch.h"
#include "VKContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "VKtypes.h"
#include "core/Platform.h"
#include "EASTL/array.h"
#include "VkUtils.h"

namespace trace {



	VKContext::VKContext()
	{
	}

	VKContext::~VKContext()
	{
	}

	void VKContext::Update(float deltaTime)
	{
	}

	void VKContext::Init()
	{

		VkResult res = vk::_CreateInstance(&m_handle);

		VK_ASSERT(res);

		TRC_INFO("Vulkan Instance Created");

#ifdef TRC_DEBUG
		vk::EnableValidationlayers(&m_handle);
		TRC_INFO("Vulkan Debug utils Created");
#endif

	}

	void VKContext::ShutDown()
	{
#ifdef TRC_DEBUG
		vk::DisableValidationlayers(&m_handle);
#endif
		vk::_DestroyInstance(&m_handle);
	}

}