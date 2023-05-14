#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "GHandle.h"

namespace trace {

	class GTexture;
	class GRenderPass;
	class GSwapchain;

	class TRACE_API GFramebuffer
	{

	public:
		GFramebuffer();
		virtual ~GFramebuffer();

		// please pass in swapchain image index and swapchain if there is going to be any swapchain image in the framebuffer
		static GFramebuffer* Create_(uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index = 0, GSwapchain* swapchain = nullptr);

		GHandle* GetRenderHandle() { return &m_renderHandle; }

	private:
		GHandle m_renderHandle;
	protected:

	};

}
