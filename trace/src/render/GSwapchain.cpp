#include "pch.h"

#include "GSwapchain.h"
#include "Renderer.h"
#include "core/platform/Vulkan/VulkanSwapchain.h"

namespace trace {




	GSwapchain::GSwapchain()
	{
	}

	GSwapchain::~GSwapchain()
	{
	}

	GSwapchain* GSwapchain::Create_(GDevice* device, GContext* context)
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
			return new VulkanSwapchain(device,context);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}

	}

