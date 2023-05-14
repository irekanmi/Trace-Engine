#pragma once

#include "render/GFramebuffer.h"
#include "core/pch.h"
#include "VKtypes.h"

namespace trace {

	class TRACE_API VulkanFramebuffer : public GFramebuffer
	{

	public:
		VulkanFramebuffer();
		VulkanFramebuffer(uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index = 0, GSwapchain* swapchain = nullptr);
		~VulkanFramebuffer();

	public:
		//VKFrameBuffer m_handle;
		eastl::vector<VKFrameBuffer> m_handle;

	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;

	protected:

	};

}

namespace vk {

	bool __CreateFrameBuffer(trace::GFramebuffer* framebuffer, uint32_t num_attachment, trace::GTexture** attachments, trace::GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index = 0, trace::GSwapchain* swapchain = nullptr);
	bool __DestroyFrameBuffer(trace::GFramebuffer* framebuffer);

}
