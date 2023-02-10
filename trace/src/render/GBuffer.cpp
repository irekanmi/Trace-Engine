#include <pch.h>

#include "GBuffer.h"
#include "GContext.h"
#include "core/platform/OpenGL/OpenGLBuffer.h"
#include "core/io/Logging.h"
#include "Renderer.h"
#include "core/platform/Vulkan/VulkanBuffer.h"

namespace trace {

	GBuffer::GBuffer()
	{

	}

	GBuffer::~GBuffer()
	{

	}

	GBuffer* GBuffer::Create_(const BufferInfo& buffer_info)
	{

		switch (Renderer::get_api())
		{
		case RenderAPI::OpenGL:
		{
			return new OpenGLBuffer(buffer_info);
			break;
		}

		case RenderAPI::Vulkan:
		{
			return new VulkanBuffer(buffer_info);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}

	void GBuffer::Create_(const BufferInfo& buffer_info, GBuffer* dst)
	{
		return;
	}


	void* GBuffer::GetNativeHandle()
	{
		return nullptr;
	}

	
}