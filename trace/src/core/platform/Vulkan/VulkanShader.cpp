#include "pch.h"

#include "VulkanShader.h"
#include "VkUtils.h"
#include "render/ShaderParser.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;

namespace trace {



	VulkanShader::VulkanShader()
		:m_handle({})
	{

		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;

	}

	VulkanShader::VulkanShader(std::string& src, ShaderStage stage)
	{
		m_stage = stage;
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;

		std::vector<uint32_t> _code = ShaderParser::glsl_to_spirv(src, stage);

		vk::_CreateShader(m_instance, m_device, &m_handle, stage, _code);

	}

	VulkanShader::~VulkanShader()
	{
		vk::_DestoryShader(m_instance, m_device, &m_handle);

	}

}