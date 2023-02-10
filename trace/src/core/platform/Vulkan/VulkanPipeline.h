#pragma once

#include "render/GPipeline.h"
#include "VKtypes.h"

namespace trace {

	class TRACE_API VulkanPipeline : public GPipeline
	{

	public:
		VulkanPipeline(PipelineStateDesc desc);
		~VulkanPipeline();

		VKPipeline m_handle;
	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;

	protected:

	};

}
