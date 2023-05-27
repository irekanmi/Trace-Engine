#pragma once

#include "render/GShader.h"
#include  "VKtypes.h"

namespace trace {

	class TRACE_API VulkanShader : public GShader
	{

	public:
		VulkanShader();
		VulkanShader(std::string& src, ShaderStage stage);
		~VulkanShader();

		
		VKShader m_handle;
	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;

	protected:

	};

}


namespace vk {

	bool __CreateShader(trace::GShader* shader, const std::string& src, trace::ShaderStage stage);
	bool __DestroyShader(trace::GShader* shader);

}