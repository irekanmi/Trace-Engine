#pragma once

#include "render/GSwapchain.h"
#include "VKtypes.h"
#include "VulkanTexture.h"

namespace trace {

	class TRACE_API VulkanSwapchain : public GSwapchain
	{

	public:
		VulkanSwapchain();
		VulkanSwapchain(GDevice* device, GContext* context);
		~VulkanSwapchain();

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Present() override;
		virtual GTexture* GetBackColorBuffer() override;
		virtual GTexture* GetBackDepthBuffer() override;

		bool Recreate();

		VKHandle* m_instance;
		VKDeviceHandle* m_device;
		VKSwapChain m_handle;
		VulkanTexture m_colorAttachment;
		VulkanTexture m_depthAttachment;
		uint32_t m_currentImageIndex;
		uint32_t m_width;
		uint32_t m_height;
		bool m_recreating;
	private:

	protected:


	};

}

namespace vk {

	bool __CreateSwapchain(trace::GSwapchain* swapchain, trace::GDevice* device, trace::GContext* context);
	bool __DestroySwapchain(trace::GSwapchain* swapchain);
	bool __ResizeSwapchain(trace::GSwapchain* swapchain, uint32_t width, uint32_t height);
	bool __PresentSwapchain(trace::GSwapchain* swapchain);
	bool __GetSwapchainColorBuffer(trace::GSwapchain* swapchain, trace::GTexture* out_texture);
	bool __GetSwapchainDepthBuffer(trace::GSwapchain* swapchain, trace::GTexture* out_texture);
	bool _recreate_swapchain(trace::GSwapchain* swapchain);
}
