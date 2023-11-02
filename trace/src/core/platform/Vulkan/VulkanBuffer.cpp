#include "pch.h"

#include "VulkanBuffer.h"
#include "VkUtils.h"


extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;


namespace vk {

	bool __CreateBuffer(trace::GBuffer* buffer, trace::BufferInfo _info)
	{
		bool result = true;

		

		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (buffer->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("These handle is valid can't recreate the buffer ::Try to destroy and then create, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKBuffer* internal_handle = new trace::VKBuffer(); //TODO: Use custom allocator;
		internal_handle->m_device = &g_VkDevice;
		internal_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = &g_Vkhandle;
		trace::VKDeviceHandle* _device = &g_VkDevice;
		buffer->GetRenderHandle()->m_internalData = internal_handle;


		VkResult buffer_create_result;

		buffer_create_result = vk::_CreateBuffer(_instance, _device, internal_handle, _info);

		if (buffer_create_result == VK_SUCCESS)
		{
			TRC_INFO(" Buffer creation successful ");
		}

		if (_info.m_data != nullptr && _info.m_usageFlag != trace::UsageFlag::UPLOAD)
		{

			trace::VKBuffer stage_buffer;

			trace::BufferInfo stage_info;
			stage_info.m_size = _info.m_size;
			stage_info.m_stide = _info.m_stide;
			stage_info.m_usageFlag = trace::UsageFlag::UPLOAD;
			stage_info.m_flag = trace::BindFlag::NIL;
			stage_info.m_data = nullptr;

			vk::_CreateBuffer(_instance, _device, &stage_buffer, stage_info);

			void* data0;
			vkMapMemory(_device->m_device, stage_buffer.m_memory, 0, stage_info.m_size, 0, &data0);
			memcpy(data0, _info.m_data, _info.m_size);
			vkUnmapMemory(_device->m_device, stage_buffer.m_memory);


			vk::_CopyBuffer(_instance, _device, &stage_buffer, internal_handle, stage_info.m_size, 0);

			vk::_DestoryBuffer(_instance, _device, &stage_buffer);
		}

		if (_info.m_usageFlag == trace::UsageFlag::UPLOAD)
		{
			vkMapMemory(_device->m_device, internal_handle->m_memory, 0, _info.m_size, VK_NO_FLAGS, &internal_handle->data_point);
		}

		buffer->m_info = _info;

		if (buffer_create_result != VK_SUCCESS) result = false;
		return result;
	}
	bool __DestroyBuffer(trace::GBuffer* buffer)
	{

		bool result = true;

		

		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKBuffer* _handle = (trace::VKBuffer*)buffer->GetRenderHandle()->m_internalData;
		trace::VKDeviceHandle* _device = reinterpret_cast<trace::VKDeviceHandle*>(_handle->m_device);
		trace::VKHandle* _instance = _handle->m_instance;

		if (buffer->m_info.m_usageFlag == trace::UsageFlag::UPLOAD)
		{
			vkUnmapMemory(_device->m_device, _handle->m_memory);
		}

		/*vkDeviceWaitIdle(_device->m_device);
		vk::_DestoryBuffer(_instance, _device, _handle);*/

		_device->frames_resources[_device->m_imageIndex]._buffers.push_back(_handle->m_handle);
		_device->frames_resources[_device->m_imageIndex]._memorys.push_back(_handle->m_memory);

		delete buffer->GetRenderHandle()->m_internalData;

		buffer->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}
	bool __SetBufferData(trace::GBuffer* buffer, void* data, uint32_t size)
	{

		bool result = true;

		

		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKBuffer* _handle = (trace::VKBuffer*)buffer->GetRenderHandle()->m_internalData;
		trace::VKDeviceHandle* _device = reinterpret_cast<trace::VKDeviceHandle*>(_handle->m_device);
		trace::VKHandle* _instance = _handle->m_instance;

		if (buffer->m_info.m_usageFlag == trace::UsageFlag::UPLOAD)
		{
			memcpy(_handle->data_point, data, size);

		}
		else
		{

			trace::VKBuffer stage_buffer;

			trace::BufferInfo stage_info;
			stage_info.m_size = static_cast<uint32_t>(size);
			stage_info.m_stide = 0;
			stage_info.m_usageFlag = trace::UsageFlag::UPLOAD;
			stage_info.m_flag = trace::BindFlag::NIL;
			stage_info.m_data = nullptr;

			vk::_CreateBuffer(_instance, _device, &stage_buffer, stage_info);

			void* data0;
			vkMapMemory(_device->m_device, stage_buffer.m_memory, 0, stage_info.m_size, 0, &data0);
			memcpy(data0, data, size);
			vkUnmapMemory(_device->m_device, stage_buffer.m_memory);


			vk::_CopyBuffer(_instance, _device, &stage_buffer, _handle, stage_info.m_size, 0);

			vk::_DestoryBuffer(_instance, _device, &stage_buffer);
		}
		return result;
	}

	bool __SetBufferDataOffset(trace::GBuffer* buffer, void* data, uint32_t offset, uint32_t size)
	{
		bool result = true;



		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKBuffer* _handle = (trace::VKBuffer*)buffer->GetRenderHandle()->m_internalData;
		trace::VKDeviceHandle* _device = reinterpret_cast<trace::VKDeviceHandle*>(_handle->m_device);
		trace::VKHandle* _instance = _handle->m_instance;

		if (buffer->m_info.m_usageFlag == trace::UsageFlag::UPLOAD)
		{
			if (offset + size <= buffer->m_info.m_size)
			{
				memcpy(((unsigned char*)_handle->data_point) + offset, data, size);
			}
			else
			{
				TRC_ASSERT(false, __FUNCTION__);
			}
		}
		else
		{
			trace::VKBuffer stage_buffer;

			trace::BufferInfo stage_info;
			stage_info.m_size = static_cast<uint32_t>(size);
			stage_info.m_stide = 0;
			stage_info.m_usageFlag = trace::UsageFlag::UPLOAD;
			stage_info.m_flag = trace::BindFlag::NIL;
			stage_info.m_data = nullptr;

			vk::_CreateBuffer(_instance, _device, &stage_buffer, stage_info);

			void* data0;
			vkMapMemory(_device->m_device, stage_buffer.m_memory, offset, stage_info.m_size, 0, &data0);
			memcpy(data0, data, size);
			vkUnmapMemory(_device->m_device, stage_buffer.m_memory);


			vk::_CopyBuffer(_instance, _device, &stage_buffer, _handle, stage_info.m_size, 0);

			vk::_DestoryBuffer(_instance, _device, &stage_buffer);

		}

		return result;
	}

	bool __CreateBatchBuffer(trace::GBuffer* buffer, trace::BufferInfo create_info)
	{
		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (buffer->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("These handle is valid can't recreate the buffer ::Try to destroy and then create, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		return true;
	}

	bool __DestroyBatchBuffer(trace::GBuffer* buffer)
	{
		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}


		return true;
	}

	bool __FlushBatchBuffer(trace::GBuffer* buffer, void* data, uint32_t size)
	{
		if (!buffer)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		

		return true;
	}

}