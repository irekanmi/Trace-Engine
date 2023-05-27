#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "GHandle.h"

namespace trace {

	class GDevice;
	class GContext;
	class GTexture;

	class TRACE_API GSwapchain
	{

	public:
		GSwapchain();
		virtual ~GSwapchain();

		virtual void Resize(uint32_t width, uint32_t height) {};
		virtual void Present() {};
		virtual GTexture* GetBackColorBuffer() { return nullptr;};
		virtual GTexture* GetBackDepthBuffer() { return nullptr;};

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		static GSwapchain* Create_(GDevice* device, GContext* context);

	private:
		GHandle m_renderHandle;
	protected:

	};

}