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
		~GSwapchain();

		void Resize(uint32_t width, uint32_t height) {};
		void Present() {};
		GTexture* GetBackColorBuffer() { return nullptr;};
		GTexture* GetBackDepthBuffer() { return nullptr;};
		uint32_t GetWidth() { return m_width; }
		uint32_t GetHeight() { return m_height; }
		void SetWidth(uint32_t width) { m_width = width; }
		void SetHeight(uint32_t height) { m_height = height; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }


	private:
		GHandle m_renderHandle;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
	protected:

	};

}