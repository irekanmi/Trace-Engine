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
