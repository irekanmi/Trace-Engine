#include "pch.h"

#include "VulkanRenderPass.h"
#include "VkUtils.h"


extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;
namespace trace {



	VulkanRenderPass::VulkanRenderPass()
	{
	}
	VulkanRenderPass::VulkanRenderPass(RenderPassDescription desc)
	{
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;
		m_desc = desc;

		/*VkResult result = VK_ERROR_INITIALIZATION_FAILED;

		uint32_t attachment_count = 2; // TODO: Configurable
		eastl::vector<VkAttachmentDescription> attachments;
		attachments.resize(attachment_count);

		VkAttachmentDescription color_attachment = {};

		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
		color_attachment.format = swapchain->m_format.format;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//TODO: Check why values can't be assigned to by indexing
		attachments[0] = color_attachment;

		VkAttachmentReference color_attach_ref = {};
		color_attach_ref.attachment = 0;
		color_attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment = {};

		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
		depth_attachment.format = device->m_depthFormat;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//TODO: Check why values can't be assigned to by indexing
		attachments[1] = (depth_attachment);

		VkAttachmentReference depth_attach_ref = {};
		depth_attach_ref.attachment = 1;
		depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// TODO: other attachent { resolve, input, preserve }

		VkSubpassDescription subpass_info = {};
		subpass_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_info.colorAttachmentCount = 1;
		subpass_info.pColorAttachments = &color_attach_ref;
		subpass_info.pDepthStencilAttachment = &depth_attach_ref;

		// TODO: Configurable
		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass = 0;
		subpass_dependency.srcAccessMask = 0;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dependencyFlags = 0;


		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &subpass_dependency;
		render_pass_info.attachmentCount = attachment_count;
		render_pass_info.pAttachments = attachments.data();
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass_info;

		result = vkCreateRenderPass(device->m_device, &render_pass_info, instance->m_alloc_callback, &render_pass->m_handle);

		VK_ASSERT(result);
		render_pass->clear_color = clear_color;
		render_pass->depth_value = depth_value;
		render_pass->stencil_value = stencil_value;
		render_pass->render_area = render_area;*/


		VkResult result;

		eastl::vector<VkSubpassDescription> subpasses;
		subpasses.resize(desc.subpass_count);
		eastl::vector<VkAttachmentDescription> attachments;
		eastl::vector<VkAttachmentReference> attachments_ref;
		VkAttachmentReference depth_attach_ref = {};
		depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		for (uint32_t i = 0; i < desc.subpass_count; i++)
		{
			attachments.resize(desc.subpasses[i].attachment_count);
			attachments_ref.resize(desc.subpasses[i].attachment_count);
			bool has_depth = false;
			
			for (uint32_t j = 0; j < desc.subpasses[i].attachment_count; j++)
			{
				attachments[j].samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
				attachments[j].format = vk::convertFmt(desc.subpasses[i].attachments[j].attachment_format);
				attachments[j].initialLayout = vk::convertTextureFmt(desc.subpasses[i].attachments[j].initial_format);
				attachments[j].finalLayout = vk::convertTextureFmt(desc.subpasses[i].attachments[j].final_format);
				attachments[j].loadOp = vk::convertAttachmentLoadOp(desc.subpasses[i].attachments[j].load_operation);
				attachments[j].storeOp = vk::convertAttachmentStoreOp(desc.subpasses[i].attachments[j].store_operation);
				attachments[j].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[j].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				if (desc.subpasses[i].attachments[j].is_depth)
				{
					has_depth = true;
					depth_attach_ref.attachment = desc.subpasses[i].attachments[j].attachmant_index;
					continue;

				}
				attachments_ref[j].attachment = desc.subpasses[i].attachments[j].attachmant_index;
				attachments_ref[j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			subpasses[i].colorAttachmentCount = static_cast<uint32_t>(attachments_ref.size() - has_depth);
			subpasses[i].pColorAttachments = attachments_ref.data();
			subpasses[i].pDepthStencilAttachment = has_depth ? &depth_attach_ref : nullptr;

		}



		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		create_info.pAttachments = attachments.data();
		create_info.subpassCount = static_cast<uint32_t>(subpasses.size());
		create_info.pSubpasses = subpasses.data();


		result = vkCreateRenderPass(
			m_device->m_device,
			&create_info,
			m_instance->m_alloc_callback,
			&m_handle.m_handle
		);

		VK_ASSERT(result);
		
		m_handle.clear_color = &m_desc.clear_color;
		m_handle.depth_value = &m_desc.depth_value;
		m_handle.stencil_value = &m_desc.stencil_value;
		m_handle.render_area = &m_desc.render_area;


	}
	VulkanRenderPass::~VulkanRenderPass()
	{
		vkDestroyRenderPass(
			m_device->m_device,
			m_handle.m_handle,
			m_instance->m_alloc_callback
		);
	}


}