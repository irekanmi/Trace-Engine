#include "pch.h"

#include "VulkanBuffer.h"

namespace trace {



	VulkanBuffer::VulkanBuffer()
	{
	}

	VulkanBuffer::VulkanBuffer(BufferInfo info)
	{
		m_info = info;
	}

	VulkanBuffer::~VulkanBuffer()
	{
	}

	void* VulkanBuffer::GetNativeHandle()
	{
		return &m_handle;
	}

	void VulkanBuffer::SetData(void* data, size_t size)
	{
	}

	void VulkanBuffer::Bind()
	{
	}

}