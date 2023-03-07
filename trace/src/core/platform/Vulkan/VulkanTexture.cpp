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

		VkImageUsageFlags image_usage = 0;
		VkImageAspectFlags aspect_flags = 0;
		VkMemoryPropertyFlags memory_property = 0;
		VkImageCreateFlags flags = 0;
		if (TRC_HAS_FLAG(desc.m_flag, BindFlag::SHADER_RESOURCE_BIT))
		{
			image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (TRC_HAS_FLAG(desc.m_flag, BindFlag::DEPTH_STENCIL_BIT))
		{
			image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (desc.m_usage == UsageFlag::DEFAULT)
		{
			memory_property |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		if (desc.m_image_type == ImageType::CUBE_MAP)
		{
			flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		vk::_CreateImage(
			m_instance,
			m_device,
			&m_handle,
			vk::convertFmt(desc.m_format),
			VK_IMAGE_TILING_OPTIMAL,
			image_usage,
			memory_property,
			aspect_flags,
			flags,
			vk::convertImageType(desc.m_image_type),
			desc.m_width,
			desc.m_height,
			desc.m_numLayers,
			1
		);

		if (!desc.m_data.empty())
		{
			VKBuffer staging_buffer;

			BufferInfo buffer_info;
			buffer_info.m_size = desc.m_width * desc.m_height * desc.m_channels;
			buffer_info.m_stide = 0;
			buffer_info.m_usageFlag = UsageFlag::UPLOAD;
			buffer_info.m_flag = BindFlag::NIL;
			buffer_info.m_data = desc.m_data[0];

			vk::_CreateBuffer(
				m_instance,
				m_device,
				&staging_buffer,
				buffer_info
			);


			for (uint32_t i = 0; i < desc.m_numLayers; i++)
			{
			VKCommmandBuffer cmd_buf;
			vk::_BeginCommandBufferSingleUse(m_device, m_device->m_graphicsCommandPool, &cmd_buf);
				VkImageSubresourceRange range = {};
				range.aspectMask = aspect_flags;
				range.baseArrayLayer = i;
				range.baseMipLevel = 0;
				range.layerCount = 1;
				range.levelCount = 1;

			vk::_TransitionImageLayout(
				m_instance,
				m_device,
				&cmd_buf,
				&m_handle,
				vk::convertFmt(desc.m_format),
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				range
			);

				void* data0;
				vkMapMemory(m_device->m_device, staging_buffer.m_memory, 0, buffer_info.m_size, 0, &data0);
				memcpy(data0, desc.m_data[i], buffer_info.m_size);
				vkUnmapMemory(m_device->m_device, staging_buffer.m_memory);

				VkBufferImageCopy copy = {};
				copy.bufferOffset = 0;
				copy.bufferRowLength = 0;
				copy.bufferImageHeight = 0;

				copy.imageOffset = { 0 };
				copy.imageExtent.depth = 1;

				copy.imageExtent.width = m_handle.m_width;
				copy.imageExtent.height = m_handle.m_height;

				copy.imageSubresource.aspectMask = aspect_flags;
				copy.imageSubresource.baseArrayLayer = i;
				copy.imageSubresource.mipLevel = 0;
				copy.imageSubresource.layerCount = 1;



				vkCmdCopyBufferToImage(
					cmd_buf.m_handle,
					staging_buffer.m_handle,
					m_handle.m_handle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);

			



			vk::_TransitionImageLayout(
				m_instance,
				m_device,
				&cmd_buf,
				&m_handle,
				vk::convertFmt(desc.m_format),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				range
			);
			vk::_EndCommandBufferSingleUse(
				m_device,
				m_device->m_graphicsCommandPool,
				m_device->m_graphicsQueue,
				&cmd_buf
			);
			}


			vk::_DestoryBuffer(m_instance, m_device, &staging_buffer);

		}
		

		


		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.format = vk::convertFmt(desc.m_format);
		create_info.viewType = vk::convertImageViewType(desc.m_image_type); // TODO: Configurable view type
		create_info.image = m_handle.m_handle;
		create_info.subresourceRange.aspectMask = aspect_flags;
		create_info.subresourceRange.baseArrayLayer = 0; // TODO: Configurable array layer
		create_info.subresourceRange.baseMipLevel = 0; // TODO: Configurable
		create_info.subresourceRange.layerCount = desc.m_numLayers;
		create_info.subresourceRange.levelCount = 1; // TODO: Configurable

		VkResult result = (vkCreateImageView(m_device->m_device, &create_info, m_instance->m_alloc_callback, &m_handle.m_view));

		VK_ASSERT(result);

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
