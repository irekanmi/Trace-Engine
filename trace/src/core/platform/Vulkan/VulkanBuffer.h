#pragma once

#include "render/GBuffer.h"
#include "VKtypes.h"

namespace trace {

	class TRACE_API VulkanBuffer : public GBuffer
	{

	public:
		VulkanBuffer();
		VulkanBuffer(BufferInfo info);
		~VulkanBuffer();

		virtual void* GetNativeHandle() override;
		virtual void SetData(void* data, size_t size) override;
		virtual void Bind() override;

		VKBuffer m_handle;
	private:
		VKHandle* m_instance;
		VKDeviceHandle* m_device;

	protected:

	};


}

namespace vk {


	bool __CreateBuffer(trace::GBuffer* buffer, trace::BufferInfo _info);
	bool __DestroyBuffer(trace::GBuffer* buffer);
	bool __SetBufferData(trace::GBuffer* buffer, void* data, uint32_t size);

}
