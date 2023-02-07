#include "pch.h"

#include "VulkanPipeline.h"

namespace trace {



	VulkanPipeline::VulkanPipeline(PipelineStateDesc desc)
	{
		m_desc = desc;
	}

	VulkanPipeline::~VulkanPipeline()
	{
	}

}