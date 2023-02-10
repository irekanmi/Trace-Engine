#include "pch.h"

#include "VulkanPipeline.h"
#include "VkUtils.h"

	extern trace::VKHandle g_Vkhandle;
	extern trace::VKDeviceHandle g_VkDevice;
namespace trace {


	VulkanPipeline::VulkanPipeline(PipelineStateDesc desc)
	{
		m_desc = desc;
		m_handle = {};
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = desc.view_port.y;
		viewport.width = desc.view_port.width;
		viewport.height = -desc.view_port.height;
		viewport.minDepth = desc.view_port.minDepth;
		viewport.maxDepth = desc.view_port.maxDepth;

		VkRect2D scissor = {};
		scissor.offset.x = scissor.offset.y = 0;
		scissor.extent.width = desc.view_port.width;
		scissor.extent.height = desc.view_port.height;

		VkResult pipeline_result = vk::_CreatePipeline(
			m_instance,
			m_device,
			1,
			&viewport,
			1,
			&scissor,
			desc,
			&m_handle
		);

		if (pipeline_result == VK_SUCCESS)
		{
			TRC_INFO("Pipeline created  |__// ...");
		}
	}

	VulkanPipeline::~VulkanPipeline()
	{
		vkDeviceWaitIdle(m_device->m_device);
		vk::_DestroyPipeline(m_instance, m_device, &m_handle);
	}

}