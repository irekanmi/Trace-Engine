#pragma once

#include "core/Core.h"
#include "GBuffer.h"


namespace trace {


	class TRACE_API GDevice
	{

	public:
		GDevice();
		virtual ~GDevice();

		virtual void Init() = 0;
		virtual void DrawElements(GBuffer* vertex_buffer) = 0;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) = 0;
		virtual void DrawIndexed(GBuffer* index_buffer) = 0;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) = 0;
		virtual void ShutDown() = 0;

		virtual void* GetNativePtr() { return nullptr; }

	private:
	protected:

	};

}
