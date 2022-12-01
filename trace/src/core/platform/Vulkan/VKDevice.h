#pragma once

#include "core/Enums.h"
#include "render/GPUtypes.h"
#include "render/GDevice.h"
#include "VKtypes.h"

namespace trace {

	class VKDevice : public GDevice
	{

	public:
		VKDevice();
		~VKDevice();

		virtual void Init() override;
		virtual void DrawElements(GBuffer* vertex_buffer) override;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) override;
		virtual void DrawIndexed(GBuffer* index_buffer) override;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) override;
		virtual void ShutDown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

	private:
		VkDeviceHandle* m_handle;
		VkHandle* m_instance;
		VkSwapChain m_swapChain;

	protected:

	};

}
