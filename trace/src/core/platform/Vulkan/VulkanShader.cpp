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

		std::vector<std::pair<std::string, int>> data_index;
		std::vector<uint32_t> _code = trace::ShaderParser::glsl_to_spirv(src, stage, data_index);
		__CreateShader_(shader, _code, stage);
		


		return result;
	}
	bool __CreateShader_(trace::GShader* shader, std::vector<uint32_t>& src, trace::ShaderStage stage)
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

		shader->SetCode(src);

		VkShaderModuleCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = src.size() * sizeof(uint32_t);
		create_info.pCode = src.data();

		VkResult _result = vkCreateShaderModule(_device->m_device, &create_info, _instance->m_alloc_callback, &_handle->m_module);


		VK_ASSERT(_result);

		_handle->create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		_handle->create_info.module = _handle->m_module;
		_handle->create_info.pName = "main";
		_handle->create_info.stage = convertShaderStage(stage);
		_handle->create_info.pSpecializationInfo = nullptr; // TODO: Check Docs for more info


		if (_result != VK_SUCCESS)
		{
			delete _handle;
			shader->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}

		return true;
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
		shader->GetCode().clear();

		delete shader->GetRenderHandle()->m_internalData;
		shader->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}

}
