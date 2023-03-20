#include "pch.h"

#include "GRenderPass.h"
#include "core/platform/Vulkan/VulkanRenderPass.h"

namespace trace {
	GRenderPass::GRenderPass()
	{

	}

	GRenderPass::~GRenderPass()
	{

	}

	GRenderPass* GRenderPass::Create_(RenderPassDescription desc)
	{
		switch (AppSettings::graphics_api)
		{
		case RenderAPI::OpenGL:
		{
			TRC_ASSERT(false, "OpenGl pipeline has not being implemented");
			return nullptr;
			break;
		}

		case RenderAPI::Vulkan:
		{
			return new VulkanRenderPass(desc);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}

}

