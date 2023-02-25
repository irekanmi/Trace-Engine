#include "pch.h"

#include "GFramebuffer.h"
#include "Renderer.h"
#include "core/platform/Vulkan/VulkanFramebuffer.h"


namespace trace {

	GFramebuffer::GFramebuffer()
	{

	}

	GFramebuffer::~GFramebuffer()
	{

	}

	GFramebuffer* GFramebuffer::Create_(uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index, GSwapchain* swapchain)
	{
		switch (Renderer::get_api())
		{
		case RenderAPI::OpenGL:
		{
			TRC_ASSERT(false, "OpenGl pipeline has not being implemented");
			return nullptr;
			break;
		}

		case RenderAPI::Vulkan:
		{
			return new VulkanFramebuffer(num_attachment, attachments, render_pass, width, height, swapchain_image_index, swapchain);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}

}