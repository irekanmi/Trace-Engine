#include "pch.h"
#include "VulkanTexture.h"
#include "VkUtils.h"
#include "render/Graphics.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;


namespace vk {

	bool __CreateTexture(trace::GTexture* texture, trace::TextureDesc desc)
	{
		bool result = true;

		

		if (!texture)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)texture, __FUNCTION__);
			return false;
		}

		if (texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These handle is valid can't recreate the texture ::Try to destroy and then create {}, Function -> {}", (const void*)texture->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKImage* _handle = new trace::VKImage(); //TODO: Use a custom allocator
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		texture->GetRenderHandle()->m_internalData = _handle;

		texture->SetTextureDescription(desc);


		VkImageUsageFlags image_usage = 0;
		VkImageAspectFlags aspect_flags = 0;
		VkMemoryPropertyFlags memory_property = 0;
		VkImageCreateFlags flags = 0;
		if (TRC_HAS_FLAG(desc.m_flag, trace::BindFlag::SHADER_RESOURCE_BIT))
		{
			image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (TRC_HAS_FLAG(desc.m_flag, trace::BindFlag::DEPTH_STENCIL_BIT))
		{
			image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (desc.m_usage == trace::UsageFlag::DEFAULT)
		{
			memory_property |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		if (desc.m_image_type == trace::ImageType::CUBE_MAP)
		{
			flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		bool has_data = false, optimal_tiling = true, linear_tiling = false;
		has_data = !desc.m_data.empty();

		VkFormatProperties fmt_prop;
		vkGetPhysicalDeviceFormatProperties(_device->m_physicalDevice, vk::convertFmt(desc.m_format), &fmt_prop);

		if (has_data)
		{
			optimal_tiling = TRC_HAS_FLAG(fmt_prop.optimalTilingFeatures, VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
			linear_tiling = TRC_HAS_FLAG(fmt_prop.linearTilingFeatures, VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
		}

		vk::_CreateImage(
			_instance,
			_device,
			_handle,
			vk::convertFmt(desc.m_format),
			optimal_tiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
			image_usage,
			memory_property,
			aspect_flags,
			flags,
			vk::convertImageType(desc.m_image_type),
			desc.m_width,
			desc.m_height,
			desc.m_numLayers,
			desc.m_mipLevels
		);

		if (has_data)
		{
			trace::VKBuffer staging_buffer;

			trace::BufferInfo buffer_info;
			buffer_info.m_size = desc.m_width * desc.m_height * trace::getFmtSize(desc.m_format);
			buffer_info.m_stide = 0;
			buffer_info.m_usageFlag = trace::UsageFlag::UPLOAD;
			buffer_info.m_flag = trace::BindFlag::NIL;
			buffer_info.m_data = desc.m_data[0];

			vk::_CreateBuffer(
				_instance,
				_device,
				&staging_buffer,
				buffer_info
			);


			for (uint32_t i = 0; i < desc.m_numLayers; i++)
			{
				trace::VKCommmandBuffer cmd_buf;
				vk::_BeginCommandBufferSingleUse(_device, _device->m_graphicsCommandPool, &cmd_buf);
				VkImageSubresourceRange range = {};
				range.aspectMask = aspect_flags;
				range.baseArrayLayer = i;
				range.baseMipLevel = 0;
				range.layerCount = 1;
				range.levelCount = 1;

				vk::_TransitionImageLayout(
					_instance,
					_device,
					&cmd_buf,
					_handle,
					vk::convertFmt(desc.m_format),
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					range
				);

				void* data0;
				vkMapMemory(_device->m_device, staging_buffer.m_memory, 0, buffer_info.m_size, 0, &data0);
				memcpy(data0, desc.m_data[i], buffer_info.m_size);
				vkUnmapMemory(_device->m_device, staging_buffer.m_memory);

				VkBufferImageCopy copy = {};
				copy.bufferOffset = 0;
				copy.bufferRowLength = 0;
				copy.bufferImageHeight = 0;

				copy.imageOffset = { 0 };
				copy.imageExtent.depth = 1;

				copy.imageExtent.width = _handle->m_width;
				copy.imageExtent.height = _handle->m_height;

				copy.imageSubresource.aspectMask = aspect_flags;
				copy.imageSubresource.baseArrayLayer = i;
				copy.imageSubresource.mipLevel = 0;
				copy.imageSubresource.layerCount = 1;



				vkCmdCopyBufferToImage(
					cmd_buf.m_handle,
					staging_buffer.m_handle,
					_handle->m_handle,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copy
				);



				if (desc.m_mipLevels > 1)
				{
					vk::_EndCommandBufferSingleUse(
						_device,
						_device->m_graphicsCommandPool,
						_device->m_graphicsQueue,
						&cmd_buf
					);
					vk::_GenerateMipLevels(
						_instance,
						_device,
						_handle,
						vk::convertFmt(desc.m_format),
						desc.m_width,
						desc.m_height,
						desc.m_mipLevels,
						i
					);
				}
				else
				{
					vk::_TransitionImageLayout(
						_instance,
						_device,
						&cmd_buf,
						_handle,
						vk::convertFmt(desc.m_format),
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						range
					);

					vk::_EndCommandBufferSingleUse(
						_device,
						_device->m_graphicsCommandPool,
						_device->m_graphicsQueue,
						&cmd_buf
					);
				}



			}


			vk::_DestoryBuffer(_instance, _device, &staging_buffer);

		}





		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.format = vk::convertFmt(desc.m_format);
		create_info.viewType = vk::convertImageViewType(desc.m_image_type); // TODO: Configurable view type
		create_info.image = _handle->m_handle;
		create_info.subresourceRange.aspectMask = aspect_flags;
		create_info.subresourceRange.baseArrayLayer = 0; // TODO: Configurable array layer
		create_info.subresourceRange.baseMipLevel = 0; // TODO: Configurable
		create_info.subresourceRange.layerCount = desc.m_numLayers;
		create_info.subresourceRange.levelCount = desc.m_mipLevels; // TODO: Configurable

		VkResult _result = (vkCreateImageView(_device->m_device, &create_info, _instance->m_alloc_callback, &_handle->m_view));

		VK_ASSERT(_result);

		vk::_CreateSampler(
			_instance,
			_device,
			desc,
			_handle->m_sampler,
			static_cast<float>(desc.m_mipLevels)
		);

		if (_result != VK_SUCCESS)
		{
			delete _handle;
			texture->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}

		return result;
	}
	bool __DestroyTexture(trace::GTexture* texture)
	{
		bool result = true;

		

		if (!texture)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)texture, __FUNCTION__);
			return false;
		}

		if (!texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)texture->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKImage* _handle = (trace::VKImage*)texture->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;

		/*vkDeviceWaitIdle(_device->m_device);
		vk::_DestroyImage(
			_instance,
			_device,
			_handle
		);

		vk::_DestroySampler(
			_instance,
			_device,
			_handle->m_sampler
		);*/

		_device->frames_resources[_device->m_imageIndex]._images.push_back(_handle->m_handle);
		_device->frames_resources[_device->m_imageIndex]._image_views.push_back(_handle->m_view);
		_device->frames_resources[_device->m_imageIndex]._samplers.push_back(_handle->m_sampler);
		_device->frames_resources[_device->m_imageIndex]._memorys.push_back(_handle->m_mem);


		delete texture->GetRenderHandle()->m_internalData;
		texture->GetRenderHandle()->m_internalData = nullptr;
		
		return result;
	}

	bool __GetTextureNativeHandle(trace::GTexture* texture, void*& out_handle)
	{
		bool result = true;

		if (!texture)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)texture, __FUNCTION__);
			return false;
		}

		if (out_handle)
		{
			TRC_ERROR("Please input pointer has a value for output handle -> {}, Function -> {}", (const void*)out_handle, __FUNCTION__);
			return false;
		}

		if (!texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)texture->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKImage* _handle = (trace::VKImage*)texture->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;

		// TODO: Check To determine maybe VkImage is to be returned
		out_handle = _handle->m_view;

		return result;
	}

	bool __GetTextureData(trace::GTexture* texture, void*& out_data)
	{
		bool result = true;

		if (!texture)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)texture, __FUNCTION__);
			return false;
		}

		if (!out_data)
		{
			TRC_ERROR("Please input pointer doesn't has a value for output handle -> {}, Function -> {}", (const void*)out_data, __FUNCTION__);
			return false;
		}

		if (!texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)texture->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKImage* _handle = (trace::VKImage*)texture->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;

		trace::TextureDesc& desc = texture->GetTextureDescription();

		trace::VKBuffer staging_buffer;

		trace::BufferInfo buffer_info;
		buffer_info.m_size = desc.m_width * desc.m_height * trace::getFmtSize(desc.m_format);
		buffer_info.m_stide = 0;
		buffer_info.m_usageFlag = trace::UsageFlag::UPLOAD;
		buffer_info.m_flag = trace::BindFlag::NIL;
		buffer_info.m_data = desc.m_data[0];

		VkImageUsageFlags image_usage = 0;
		VkImageAspectFlags aspect_flags = 0;
		VkMemoryPropertyFlags memory_property = 0;
		VkImageCreateFlags flags = 0;
		if (TRC_HAS_FLAG(desc.m_flag, trace::BindFlag::SHADER_RESOURCE_BIT))
		{
			image_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (TRC_HAS_FLAG(desc.m_flag, trace::BindFlag::DEPTH_STENCIL_BIT))
		{
			image_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		if (desc.m_usage == trace::UsageFlag::DEFAULT)
		{
			memory_property |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		if (desc.m_image_type == trace::ImageType::CUBE_MAP)
		{
			flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}


		vk::_CreateBuffer(
			_instance,
			_device,
			&staging_buffer,
			buffer_info
		);

		trace::VKCommmandBuffer cmd_buf;
		vk::_BeginCommandBufferSingleUse(_device, _device->m_graphicsCommandPool, &cmd_buf);

		VkBufferImageCopy copy = {};
		copy.imageOffset = { 0, 0, 0 };
		copy.imageExtent = { desc.m_width, desc.m_height, 1 };
		copy.imageSubresource.aspectMask = aspect_flags;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageSubresource.mipLevel = 0;
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		copy.bufferImageHeight = 0;

		vkCmdCopyImageToBuffer(
			cmd_buf.m_handle,
			_handle->m_handle,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,//TODO: Textures should know there current layout
			staging_buffer.m_handle,
			1,
			&copy
		);


		vk::_EndCommandBufferSingleUse(
			_device,
			_device->m_graphicsCommandPool,
			_device->m_graphicsQueue,
			&cmd_buf
		);

		void* data;
		vkMapMemory(_device->m_device, staging_buffer.m_memory, 0, buffer_info.m_size, 0, &data);
		memcpy(out_data, data, buffer_info.m_size);
		vkUnmapMemory(_device->m_device,staging_buffer.m_memory);


		vk::_DestoryBuffer(_instance, _device, &staging_buffer);

		return result;
	}

}
