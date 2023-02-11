#pragma once

#include "render/GTexture.h"
#include "VKtypes.h"

namespace trace {

	class TRACE_API VulkanTexture : public GTexture
	{

	public:
		VulkanTexture(TextureDesc desc);
		~VulkanTexture();


		VKImage m_handle;
		VkSampler m_sampler;
	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;

	protected:

	};

}
