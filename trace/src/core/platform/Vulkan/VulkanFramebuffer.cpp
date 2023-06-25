#include "pch.h"
#include "VulkanFramebuffer.h"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VkUtils.h"
#include "VulkanSwapchain.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;



namespace vk {

	bool __CreateFrameBuffer(trace::GFramebuffer* framebuffer, uint32_t num_attachment, trace::GTexture** attachments, trace::GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index, trace::GSwapchain* swapchain)
	{
		bool result = true;

		

		if (!framebuffer)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)framebuffer, __FUNCTION__);
			return false;
		}

		if (framebuffer->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("These handle is valid can't recreate the frame buffer ::Try to destroy and then create, {}, Function -> {}", (const void*)framebuffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::Framebuffer_VK* _handle = new trace::Framebuffer_VK();
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = _handle->m_instance;
		trace::VKDeviceHandle* _device = _handle->m_device;
		framebuffer->GetRenderHandle()->m_internalData = _handle;


		eastl::vector<VkImageView> views;
		views.resize(num_attachment);

		trace::VKRenderPass* pass = reinterpret_cast<trace::VKRenderPass*>(render_pass->GetRenderHandle()->m_internalData);

		VkResult _result;

		for (uint32_t i = 0; i < num_attachment; i++)
		{
			trace::VKImage* tex = reinterpret_cast<trace::VKImage*>(attachments[i]->GetRenderHandle()->m_internalData);
			views[i] = (tex->m_view);
		}

		trace::VKSwapChain* swap = reinterpret_cast<trace::VKSwapChain*>(swapchain->GetRenderHandle()->m_internalData);

		if (swap)
		{
			_handle->m_handle.resize(swap->image_count);

			for (uint32_t i = 0; i < swap->image_count; i++)
			{
				views[swapchain_image_index - 1] = swap->m_imageViews[i];

				_result = vk::_CreateFrameBuffer(
					_instance,
					_device,
					&_handle->m_handle[i],
					views,
					pass,
					width,
					height,
					num_attachment
				);
				VK_ASSERT(_result);
			}
		}
		else
		{
			_result = vk::_CreateFrameBuffer(
				_instance,
				_device,
				&_handle->m_handle.emplace_back(),
				views,
				pass,
				800, // TODO: Configurable
				600, // TODO: Configurable
				num_attachment
			);

		}


		VK_ASSERT(_result);
		if (_result != VK_SUCCESS)
		{
			delete _handle;
			framebuffer->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}


		return result;
	}
	bool __DestroyFrameBuffer(trace::GFramebuffer* framebuffer)
	{
		bool result = true;

		

		if (!framebuffer)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)framebuffer, __FUNCTION__);
			return false;
		}

		if (!framebuffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)framebuffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::Framebuffer_VK* _handle = (trace::Framebuffer_VK*)framebuffer->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = _handle->m_instance;
		trace::VKDeviceHandle* _device = _handle->m_device;


		vkDeviceWaitIdle(_device->m_device);
		for (auto& i : _handle->m_handle)
		{

			vk::_DestoryFrameBuffer(
				_instance,
				_device,
				&i
			);
		}


		delete _handle;
		framebuffer->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}

}