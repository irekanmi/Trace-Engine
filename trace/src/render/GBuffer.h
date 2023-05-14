#pragma once

#include "core/Core.h"
#include "Graphics.h"
#include "GHandle.h"


namespace trace {



	class TRACE_API GBuffer
	{

	public:
		GBuffer();
		virtual ~GBuffer();

		uint32_t GetSize() { return m_info.m_size; }
		uint32_t GetCount() { return m_info.m_size / m_info.m_stide; }

		virtual void* GetNativeHandle() = 0;
		virtual void SetData(void* data, size_t size) = 0;
		virtual void Bind() = 0;

		GHandle* GetRenderHandle() { return &m_renderHandle; }

	public:
		static GBuffer* Create_(const BufferInfo& buffer_info);
		static void Create_(const BufferInfo& buffer_info, GBuffer* dst);
		BufferInfo m_info;
	private:
		GHandle m_renderHandle;

	protected:
		// TODO: suggesting maybe the gpu buffer info should only debug build ==> "BufferInfo m_info"


	};


}
