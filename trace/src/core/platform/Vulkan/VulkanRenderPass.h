#pragma once

#include "render/GRenderPass.h"
#include "VKtypes.h"

namespace trace {


	class TRACE_API VulkanRenderPass : public GRenderPass
	{

	public:
		VulkanRenderPass();
		VulkanRenderPass(RenderPassDescription desc);
		~VulkanRenderPass();

	public:
		VKRenderPass m_handle;


	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;

	protected:


	};

}
