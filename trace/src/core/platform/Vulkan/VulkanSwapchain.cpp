#include "pch.h"

#include "VulkanSwapchain.h"
#include "VKDevice.h"
#include "VkUtils.h"
#include "core/Platform.h"

extern trace::VKHandle g_Vkhandle;


namespace trace {



	VulkanSwapchain::VulkanSwapchain()
	{
	}

	VulkanSwapchain::VulkanSwapchain(GDevice* device, GContext* context)
	{
		m_recreating = false;
		m_width = 800; // TODO Configurable
		m_height = 600;//TODO Configurable

		m_device = reinterpret_cast<VKDevice*>(device)->m_handle;
		m_instance = &g_Vkhandle;

		m_colorAttachment.m_device = m_device;
		m_colorAttachment.m_instance = m_instance;
		
		m_depthAttachment.m_device = m_device;
		m_depthAttachment.m_instance = m_instance;

		VkResult result = vk::_CreateSwapchain(
			m_instance,
			m_device,
			&m_handle,
			m_width,
			m_height 
		);

		if (result == VK_SUCCESS)
		{
			TRC_INFO("Swapchain created");
		}

		

	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		m_colorAttachment.m_handle = {};
		m_depthAttachment.m_handle = {};
		Platform::ZeroMem(&m_colorAttachment, sizeof(VulkanTexture));
		Platform::ZeroMem(&m_depthAttachment, sizeof(VulkanTexture));
		vk::_DestroySwapchain(m_instance, m_device, &m_handle);
	}

	void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
	{
		m_recreating = true;
		m_width = width;
		m_height = height;

		Recreate();
	}

	void VulkanSwapchain::Present()
	{
		vk::_PresentSwapchainImage(
			m_instance,
			m_device, 
			&m_handle, 
			m_device->m_graphicsQueue, 
			m_device->m_presentQueue, 
			m_device->m_queueCompleteSemaphores[m_device->m_currentFrame], 
			&m_device->m_imageIndex
		);

	}

	GTexture* VulkanSwapchain::GetBackColorBuffer()
	{
		m_colorAttachment.m_handle.m_handle = m_handle.m_images[m_device->m_imageIndex];
		m_colorAttachment.m_handle.m_view = m_handle.m_imageViews[m_device->m_imageIndex];
		return &m_colorAttachment;
	}

	GTexture* VulkanSwapchain::GetBackDepthBuffer()
	{
		m_depthAttachment.m_handle = m_handle.m_depthimage;
		return &m_depthAttachment;
	}

	bool VulkanSwapchain::Recreate()
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_physicalDevice, m_instance->m_surface, &m_device->m_swapchainInfo.surface_capabilities);


		VkResult result = vk::_RecreateSwapchain(
			m_instance,
			m_device,
			&m_handle,
			m_width,
			m_height
		);

		if (m_width == 0 || m_height == 0)
		{
			return false;
		}

		if (result != VK_SUCCESS)
		{
			return false;
		}

		m_recreating = false;

		return true;
	}

}