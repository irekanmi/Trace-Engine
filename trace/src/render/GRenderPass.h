#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "RenderPassInfo.h"
#include "GHandle.h"

namespace trace {

	class TRACE_API GRenderPass
	{
	public:
		GRenderPass();
		virtual ~GRenderPass();

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		static GRenderPass* Create_(RenderPassDescription desc);

	public:
		RenderPassDescription m_desc;

	private:
		GHandle m_renderHandle;
	protected:

	};

}
