#include "pch.h"

#include "GTexture.h"
#include "core/io/Logging.h"
#include "core/platform/Vulkan/VulkanTexture.h"

namespace trace {



	GTexture* GTexture::Create_(TextureDesc description)
	{
		switch (AppSettings::graphics_api)
		{
		case RenderAPI::OpenGL:
		{
			TRC_ASSERT(false, "OpenGl Texture has not being implemented");
			return nullptr;
			break;
		}

		case RenderAPI::Vulkan:
		{
			return new VulkanTexture(description);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}

}