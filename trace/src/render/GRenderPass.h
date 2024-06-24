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

		RenderPassDescription& GetRenderPassDescription() { return m_desc; };
		uint32_t GetColorAttachmentCount() { return m_colorAttachmentCount; }


		void SetRenderPassDescription(RenderPassDescription& desc) { m_desc = desc; };
		void SetColorAttachmentCount(uint32_t attachment_count) { m_colorAttachmentCount = attachment_count; }

	private:
		RenderPassDescription m_desc;
		uint32_t m_colorAttachmentCount;
		GHandle m_renderHandle;
	protected:

	};

}
