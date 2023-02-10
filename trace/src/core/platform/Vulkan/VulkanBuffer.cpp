#include "pch.h"

#include "VulkanBuffer.h"
#include "VkUtils.h"


extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;
namespace trace {



	VulkanBuffer::VulkanBuffer()
	{
	}

	VulkanBuffer::VulkanBuffer(BufferInfo info)
	{
		m_info = info;
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;


		VkResult buffer_create_result;

		buffer_create_result = vk::_CreateBuffer(m_instance, m_device, &m_handle, info);

		if (buffer_create_result == VK_SUCCESS)
		{
			TRC_INFO(" Buffer creation successful ");
		}

		if (info.m_data != nullptr)
		{

			VKBuffer stage_buffer;

			BufferInfo stage_info;
			stage_info.m_size = info.m_size;
			stage_info.m_stide = info.m_stide;
			stage_info.m_usage = BufferUsage::STAGING_BUFFER;
			stage_info.m_data = nullptr;

			vk::_CreateBuffer(m_instance, m_device, &stage_buffer, stage_info);

			void* data0;
			vkMapMemory(m_device->m_device, stage_buffer.m_memory, 0, stage_info.m_size, 0, &data0);
			memcpy(data0, info.m_data, info.m_size);
			vkUnmapMemory(m_device->m_device, stage_buffer.m_memory);


			vk::_CopyBuffer(m_instance, m_device, &stage_buffer, &m_handle, stage_info.m_size, 0);

			vk::_DestoryBuffer(m_instance, m_device, &stage_buffer);
		}


	}

	VulkanBuffer::~VulkanBuffer()
	{
		vkDeviceWaitIdle(m_device->m_device);
		vk::_DestoryBuffer(m_instance, m_device, &m_handle);
	}

	void* VulkanBuffer::GetNativeHandle()
	{
		return &m_handle;
	}

	void VulkanBuffer::SetData(void* data, size_t size)
	{

		VKBuffer stage_buffer;

		BufferInfo stage_info;
		stage_info.m_size = size;
		stage_info.m_stide = 0;
		stage_info.m_usage = BufferUsage::STAGING_BUFFER;
		stage_info.m_data = nullptr;

		vk::_CreateBuffer(m_instance, m_device, &stage_buffer, stage_info);

		void* data0;
		vkMapMemory(m_device->m_device, stage_buffer.m_memory, 0, stage_info.m_size, 0, &data0);
		memcpy(data0, data, size);
		vkUnmapMemory(m_device->m_device, stage_buffer.m_memory);


		vk::_CopyBuffer(m_instance, m_device, &stage_buffer, &m_handle, stage_info.m_size, 0);

		vk::_DestoryBuffer(m_instance, m_device, &stage_buffer);

	}

	void VulkanBuffer::Bind()
	{
	}

}