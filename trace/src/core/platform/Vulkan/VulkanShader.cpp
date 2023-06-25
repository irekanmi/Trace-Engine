#include "pch.h"

#include "VulkanShader.h"
#include "VkUtils.h"
#include "render/ShaderParser.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;


namespace vk {

	bool __CreateShader(trace::GShader* shader, const std::string& src, trace::ShaderStage stage)
	{
		bool result = true;

		

		if (!shader)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)shader, __FUNCTION__);
			return false;
		}

		if (shader->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These handle is valid can't recreate the shader ::Try to destroy and then create, {}, Function -> {}", (const void*)shader->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKShader* _handle = new trace::VKShader(); //TODO: Use a custom allocator
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		shader->GetRenderHandle()->m_internalData = _handle;


		shader->SetShaderStage(stage);


		std::vector<uint32_t> _code = trace::ShaderParser::glsl_to_spirv(src, stage);

		VkResult _result  = vk::_CreateShader(_instance, _device, _handle, stage, _code);

		if (_result != VK_SUCCESS)
		{
			delete _handle;
			shader->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}


		return result;
	}
	bool __DestroyShader(trace::GShader* shader)
	{
		bool result = true;

		

		if (!shader)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)shader, __FUNCTION__);
			return false;
		}

		if (!shader->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)shader->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKShader* _handle = (trace::VKShader*)shader->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;

		vk::_DestoryShader(_instance, _device, _handle);

		delete shader->GetRenderHandle()->m_internalData;
		shader->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}

}
