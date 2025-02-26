#include "pch.h"

#include "VulkanSwapchain.h"
#include "VKDevice.h"
#include "VKContext.h"
#include "VkUtils.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;




namespace vk {

	bool __CreateSwapchain(trace::GSwapchain* swapchain, trace::GDevice* device, trace::GContext* context)
	{
		bool result = true;

		

		if (!swapchain || !device || !context)
		{
			TRC_ERROR("Please input valid pointer -> {} || {} || {}, Function -> {}", (const void*)swapchain, (const void*)device, (const void*)context, __FUNCTION__);
			return false;
		}

		if (swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("These handle is valid can't recreate the swapchain ::Try to destroy and then create, -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !context->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {} || {} || {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, (const void*)device->GetRenderHandle()->m_internalData, (const void*)context->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = new trace::VKSwapChain(); //TODO: Use a custom allocator
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		swapchain->GetRenderHandle()->m_internalData = _handle;

		_handle->m_recreating = false;
		_handle->m_width = 800; // TODO Configurable
		_handle->m_height = 600;//TODO Configurable
		swapchain->SetWidth(800);
		swapchain->SetHeight(600);



		VkResult _result = vk::_CreateSwapchain(
			_instance,
			_device,
			_handle,
			_handle->m_width,
			_handle->m_height
		);

		if (_result == VK_SUCCESS)
		{
			TRC_INFO("Swapchain created");
		}
		else
		{
			delete _handle;
			swapchain->GetRenderHandle()->m_internalData = nullptr;
			TRC_ERROR("Failed to create swapchain, {}", (const void*)swapchain);
			result = false;
		}

		return result;
	}
	bool __DestroySwapchain(trace::GSwapchain* swapchain)
	{
		bool result = true;

		

		if (!swapchain)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = (trace::VKSwapChain*)swapchain->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


		vk::_DestroySwapchain(_instance, _device, _handle);


		delete swapchain->GetRenderHandle()->m_internalData;
		swapchain->GetRenderHandle()->m_internalData = nullptr;
		return result;
	}
	bool __ResizeSwapchain(trace::GSwapchain* swapchain, uint32_t width, uint32_t height)
	{
		bool result = true;

		

		if (!swapchain)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = (trace::VKSwapChain*)swapchain->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;

		_handle->m_recreating = true;
		_handle->m_width = width;
		_handle->m_height = height;
		swapchain->SetWidth(width);
		swapchain->SetHeight(height);

		_recreate_swapchain(swapchain);


		return result;
	}
	bool __PresentSwapchain(trace::GSwapchain* swapchain)
	{
		bool result = true;

		

		if (!swapchain)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = (trace::VKSwapChain*)swapchain->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		vk::_PresentSwapchainImage(
			_instance,
			_device,
			_handle,
			_device->m_graphicsQueue,
			_device->m_presentQueue,
			_device->m_queueCompleteSemaphores[_device->m_currentFrame],
			&_device->m_imageIndex
		);

		return result;
	}
	bool __GetSwapchainColorBuffer(trace::GSwapchain* swapchain, trace::GTexture* out_texture)
	{
		bool result = true;

		

		if (!swapchain)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = (trace::VKSwapChain*)swapchain->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


		if (out_texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Please input a texture with an invalid handle, {}, Function -> {}", (const void*)out_texture, __FUNCTION__);
			return false;
		}

		_handle->tempColor.m_handle = _handle->m_images[_device->m_imageIndex];
		_handle->tempColor.m_view = _handle->m_imageViews[_device->m_imageIndex];

		out_texture->GetRenderHandle()->m_internalData = &_handle->tempColor;



		return result;
	}
	bool __GetSwapchainDepthBuffer(trace::GSwapchain* swapchain, trace::GTexture* out_texture)
	{
		bool result = true;

		

		if (!swapchain)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = (trace::VKSwapChain*)swapchain->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;

		if (out_texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Please input a texture with an invalid handle, {}, Function -> {}", (const void*)out_texture, __FUNCTION__);
			return false;
		}


		out_texture->GetRenderHandle()->m_internalData = &_handle->m_depthimage;


		return result;
	}
	bool _recreate_swapchain(trace::GSwapchain* swapchain)
	{
		bool result = true;

		

		if (!swapchain)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKSwapChain* _handle = (trace::VKSwapChain*)swapchain->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->m_physicalDevice, _instance->m_surface, &_device->m_swapchainInfo.surface_capabilities);

		if (_handle->m_width == 0 || _handle->m_height == 0)
		{
			return false;
		}

		VkResult _result = vk::_RecreateSwapchain(
			_instance,
			_device,
			_handle,
			_handle->m_width,
			_handle->m_height
		);



		if (_result != VK_SUCCESS)
		{
			return false;
		}

		_handle->m_recreating = false;


		return result;
	}
}