#pragma once

#include "render/GBuffer.h"
#include "VKtypes.h"


namespace vk {


	bool __CreateBuffer(trace::GBuffer* buffer, trace::BufferInfo _info);
	bool __DestroyBuffer(trace::GBuffer* buffer);
	bool __SetBufferData(trace::GBuffer* buffer, void* data, uint32_t size);
	bool __SetBufferDataOffset(trace::GBuffer* buffer, void* data, uint32_t offset, uint32_t size);
	bool __CreateBatchBuffer(trace::GBuffer* buffer, trace::BufferInfo create_info);
	bool __DestroyBatchBuffer(trace::GBuffer* buffer);
	bool __FlushBatchBuffer(trace::GBuffer* buffer, void* data, uint32_t size);

}
