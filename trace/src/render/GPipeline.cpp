#include "pch.h"

#include "GPipeline.h"
#include "Renderer.h"
#include "core/io/Logging.h"
#include "core/platform/Vulkan/VulkanPipeline.h"

namespace trace {



	GPipeline::GPipeline()
	{
	}

	GPipeline::~GPipeline()
	{
	}

	GPipeline* GPipeline::Create_(PipelineStateDesc desc)
	{
		switch (Renderer::get_api())
		{
		case RenderAPI::OpenGL:
		{
			TRC_ASSERT(false, "OpenGl pipeline has not being implemented");
			return nullptr;
			break;
		}

		case RenderAPI::Vulkan:
		{
			return new VulkanPipeline(desc);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}

}