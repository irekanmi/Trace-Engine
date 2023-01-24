#include "pch.h"
#include "VKContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "VKtypes.h"
#include "core/Platform.h"
#include "EASTL/array.h"
#include "VkUtils.h"

trace::VKHandle g_Vkhandle;
namespace trace {


	VKContext::VKContext()
	{
		m_handle = &g_Vkhandle;
	}

	VKContext::~VKContext()
	{
	}

	void VKContext::Update(float deltaTime)
	{
	}

	void VKContext::Init()
	{

		VkResult res = vk::_CreateInstance(m_handle);

		VK_ASSERT(res);

		TRC_INFO("Vulkan Instance Created");

#ifdef TRC_DEBUG_BUILD
		vk::EnableValidationlayers(m_handle);
		TRC_INFO("Vulkan Debug utils Created");
#endif

		VK_ASSERT(vk::_CreateSurface(m_handle));

	}

	void VKContext::ShutDown()
	{
		vk::_DestorySurface(m_handle);

#ifdef TRC_DEBUG_BUILD
		vk::DisableValidationlayers(m_handle);
#endif
		vk::_DestroyInstance(m_handle);
	}

}