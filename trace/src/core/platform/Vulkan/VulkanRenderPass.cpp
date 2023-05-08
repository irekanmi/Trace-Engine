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

		//Test =========================================
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		//==============================================

		VkRenderPassCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		create_info.pAttachments = attachments.data();
		create_info.subpassCount = static_cast<uint32_t>(subpasses.size());
		create_info.pSubpasses = subpasses.data();

		//Test =================================
		create_info.dependencyCount = 1;
		create_info.pDependencies = &dependency;
		//==============================


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