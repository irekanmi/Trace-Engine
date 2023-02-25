#include "pch.h"
#include "VulkanFramebuffer.h"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VkUtils.h"
#include "VulkanSwapchain.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;

namespace trace {
	VulkanFramebuffer::VulkanFramebuffer()
	{
	}

	VulkanFramebuffer::VulkanFramebuffer(uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index, GSwapchain* swapchain)
	{
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;

		eastl::vector<VkImageView> views;
		views.resize(num_attachment);

		VulkanRenderPass* pass = reinterpret_cast<VulkanRenderPass*>(render_pass);

		VkResult result;

		for (uint32_t i = 0; i < num_attachment; i++)
		{
			VulkanTexture* tex = reinterpret_cast<VulkanTexture*>(attachments[i]);
			views[i] = (tex->m_handle.m_view);
		}

		VulkanSwapchain* swap = reinterpret_cast<VulkanSwapchain*>(swapchain);

		if (swap)
		{
			m_handle.resize(swap->m_handle.image_count);

			for (uint32_t i = 0; i < swap->m_handle.image_count; i++)
			{
				views[swapchain_image_index - 1] = swap->m_handle.m_imageViews[i];

				result = vk::_CreateFrameBuffer(
					m_instance,
					m_device,
					&m_handle[i],
					views,
					&pass->m_handle,
					width,
					height,
					num_attachment
				);
				VK_ASSERT(result);
			}
		}
		else
		{
			result = vk::_CreateFrameBuffer(
				m_instance,
				m_device,
				&m_handle.emplace_back(),
				views,
				&pass->m_handle,
				800, // TODO: Configurable
				600, // TODO: Configurable
				num_attachment
			);

		}


		VK_ASSERT(result);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{

		vkDeviceWaitIdle(m_device->m_device);
		for (auto& i : m_handle)
		{

			vk::_DestoryFrameBuffer(
				m_instance,
				m_device,
				&i
			);
		}

	}

}
