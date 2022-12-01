#include "pch.h"

#include "VKDevice.h"
#include "VKtypes.h"
#include "VkUtils.h"
#include "core/Application.h"

extern trace::VkHandle g_Vkhandle;
trace::VkDeviceHandle g_VkDevice;
namespace trace {


	VKDevice::VKDevice()
	{
		m_instance = &g_Vkhandle;
		m_handle = &g_VkDevice;
	}

	VKDevice::~VKDevice()
	{
	}

	void VKDevice::Init()
	{
		VK_ASSERT(vk::_CreateDevice(m_handle, m_instance));

		TRC_INFO("Created Vulkan Device");

		TRC_TRACE("Creating Swapchain...");
		VK_ASSERT(vk::_CreateSwapchain(m_instance, m_handle, &m_swapChain, Application::get_instance()->GetWindow()->GetWidth(), Application::get_instance()->GetWindow()->GetHeight()));

		TRC_INFO("Vulkan SwapChain Created");
	}

	void VKDevice::DrawElements(GBuffer* vertex_buffer)
	{
	}

	void VKDevice::DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances)
	{
	}

	void VKDevice::DrawIndexed(GBuffer* index_buffer)
	{
	}

	void VKDevice::DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances)
	{
	}

	void VKDevice::ShutDown()
	{
		vk::_DestroySwapchain(m_instance, m_handle, &m_swapChain);
		vk::_DestoryDevice(m_handle, m_instance);
	}

	void VKDevice::BeginFrame()
	{
	}

	void VKDevice::EndFrame()
	{
	}

}