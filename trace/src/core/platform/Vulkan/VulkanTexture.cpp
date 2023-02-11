#include "pch.h"
#include "VulkanTexture.h"
#include "VkUtils.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;

namespace trace {
	VulkanTexture::VulkanTexture(TextureDesc desc)
	{
		m_desc = desc;
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;

		vk::_CreateImage(
			m_instance,
			m_device,
			&m_handle,
			vk::convertFmt(desc.m_format),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			desc.m_width,
			desc.m_height
		);

		VKBuffer staging_buffer;

		BufferInfo buffer_info;
		buffer_info.m_size = desc.m_width * desc.m_height * desc.m_channels;
		buffer_info.m_stide = 0;
		buffer_info.m_usage = BufferUsage::STAGING_BUFFER;
		buffer_info.m_data = desc.m_data;

		vk::_CreateBuffer(
			m_instance,
			m_device,
			&staging_buffer,
			buffer_info
		);

		vk::_CreateImageView(
			m_instance,
			m_device,
			&m_handle.m_view,
			vk::convertFmt(desc.m_format),
			VK_IMAGE_ASPECT_COLOR_BIT,
			&m_handle.m_handle
		);

		void* data0;
		vkMapMemory(m_device->m_device, staging_buffer.m_memory, 0, buffer_info.m_size, 0, &data0);
		memcpy(data0, buffer_info.m_data, buffer_info.m_size);
		vkUnmapMemory(m_device->m_device, staging_buffer.m_memory);

		VKCommmandBuffer cmd_buf;
		vk::_BeginCommandBufferSingleUse(m_device, m_device->m_graphicsCommandPool, &cmd_buf);


		vk::_TransitionImageLayout(
			m_instance,
			m_device,
			&cmd_buf,
			&m_handle,
			vk::convertFmt(desc.m_format),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);

		vk::_CopyBufferToImage(
			m_instance,
			m_device,
			&cmd_buf,
			&m_handle,
			&staging_buffer
		);

		vk::_TransitionImageLayout(
			m_instance,
			m_device,
			&cmd_buf,
			&m_handle,
			vk::convertFmt(desc.m_format),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);

		vk::_EndCommandBufferSingleUse(
			m_device,
			m_device->m_graphicsCommandPool,
			m_device->m_graphicsQueue,
			&cmd_buf
		);

		vk::_DestoryBuffer(m_instance, m_device, &staging_buffer);

		vk::_CreateSampler(
			m_instance,
			m_device,
			desc,
			m_sampler
		);

	}
	VulkanTexture::~VulkanTexture()
	{
		vk::_DestroyImage(
			m_instance,
			m_device,
			&m_handle
		);

		vk::_DestroySampler(
			m_instance,
			m_device,
			m_sampler
		);
	}
}
