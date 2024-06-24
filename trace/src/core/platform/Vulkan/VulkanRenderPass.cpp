#include "pch.h"

#include "VulkanRenderPass.h"
#include "VkUtils.h"


extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;

namespace vk {

	bool __CreateRenderPass(trace::GRenderPass* render_pass, trace::RenderPassDescription desc)
	{
		bool result = true;

		

		if (!render_pass)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)render_pass, __FUNCTION__);
			return false;
		}

		if (render_pass->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("These handle is valid can't recreate the render pass ::Try to destroy and then create, {}, Function -> {}", (const void*)render_pass->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKRenderPass* _handle = new trace::VKRenderPass(); //TODO: Use a custom allocator
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		render_pass->GetRenderHandle()->m_internalData = _handle;

		render_pass->SetRenderPassDescription(desc);

		VkResult _result;
		VkSubpassDescription subpass = {};
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> attachments_ref;
		VkAttachmentReference depth_attach_ref = {};
		depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments.resize(desc.subpass.attachment_count);
		attachments_ref.resize(desc.subpass.attachment_count);
		bool has_depth = false;

		for (uint32_t j = 0; j < desc.subpass.attachment_count; j++)
		{
			attachments[j].samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
			attachments[j].format = vk::convertFmt(desc.subpass.attachments[j].attachment_format);
			attachments[j].initialLayout = vk::convertTextureFmt(desc.subpass.attachments[j].initial_format);
			attachments[j].finalLayout = vk::convertTextureFmt(desc.subpass.attachments[j].final_format);
			attachments[j].loadOp = vk::convertAttachmentLoadOp(desc.subpass.attachments[j].load_operation);
			attachments[j].storeOp = vk::convertAttachmentStoreOp(desc.subpass.attachments[j].store_operation);
			attachments[j].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[j].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			if (desc.subpass.attachments[j].is_depth)
			{
				has_depth = true;
				depth_attach_ref.attachment = desc.subpass.attachments[j].attachmant_index;
				continue;

			}
			attachments_ref[j].attachment = desc.subpass.attachments[j].attachmant_index;
			attachments_ref[j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		subpass.colorAttachmentCount = static_cast<uint32_t>(attachments_ref.size() - has_depth);
		subpass.pColorAttachments = attachments_ref.data();
		subpass.pDepthStencilAttachment = has_depth ? &depth_attach_ref : nullptr;
		

		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		create_info.pAttachments = attachments.data();
		create_info.subpassCount = 1;
		create_info.pSubpasses = &subpass;

		_result = vkCreateRenderPass(
			_device->m_device,
			&create_info,
			_instance->m_alloc_callback,
			&_handle->m_handle
		);

		VK_ASSERT(_result);

		_handle->clear_color = &render_pass->GetRenderPassDescription().clear_color;
		_handle->depth_value = &render_pass->GetRenderPassDescription().depth_value;
		_handle->stencil_value = &render_pass->GetRenderPassDescription().stencil_value;
		_handle->render_area = &render_pass->GetRenderPassDescription().render_area;

		if (_result != VK_SUCCESS)
		{
			delete _handle;
			render_pass->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}
		
		render_pass->SetColorAttachmentCount(static_cast<uint32_t>(attachments_ref.size() - has_depth));

		return result;
	}
	bool __DestroyRenderPass(trace::GRenderPass* render_pass)
	{
		bool result = true;

		

		if (!render_pass)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)render_pass, __FUNCTION__);
			return false;
		}

		if (!render_pass->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)render_pass->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKRenderPass* _handle = (trace::VKRenderPass*)render_pass->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


		vkDestroyRenderPass(
			_device->m_device,
			_handle->m_handle,
			_instance->m_alloc_callback
		);

		delete render_pass->GetRenderHandle()->m_internalData;
		render_pass->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}

}