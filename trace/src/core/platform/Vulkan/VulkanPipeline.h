#pragma once

#include "render/GPipeline.h"
#include "VKtypes.h"

namespace trace {

	class TRACE_API VulkanPipeline : public GPipeline
	{

	public:
		VulkanPipeline(PipelineStateDesc desc);
		~VulkanPipeline();

	private:
	protected:

	};

}
