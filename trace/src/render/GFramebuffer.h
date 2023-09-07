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


		GHandle* GetRenderHandle() { return &m_renderHandle; }

	private:
		GHandle m_renderHandle;
	protected:

	};

}
