#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "RenderPassInfo.h"

namespace trace {

	class TRACE_API GRenderPass
	{
	public:
		GRenderPass();
		virtual ~GRenderPass();


		static GRenderPass* Create_(RenderPassDescription desc);

	public:
		RenderPassDescription m_desc;

	private:
	protected:

	};

}
