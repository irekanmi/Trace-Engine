#include "pch.h"

#include "GShader.h"
#include "core/io/Logging.h"
#include "core/platform/Vulkan/VulkanShader.h"

namespace trace {
	GShader::GShader()
	{
	}
	GShader::~GShader()
	{
	}
	GShader* GShader::Create_(std::string& src, ShaderStage stage)
	{
		switch (AppSettings::graphics_api)
		{
		case RenderAPI::OpenGL:
		{
			TRC_ASSERT(false, "OpenGl Shader has not being implemented");
			return nullptr;
			break;
		}

		case RenderAPI::Vulkan:
		{
			return new VulkanShader(src, stage);
			break;
		}

		}

		TRC_ASSERT(false, "Render API can't be null");
		return nullptr;
	}
	GShader* GShader::Create_(FileHandle& file)
	{
		return nullptr;
	}
}