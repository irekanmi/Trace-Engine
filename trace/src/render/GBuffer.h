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

		virtual void* GetNativeHandle();
		virtual void SetData(void* data, size_t size) {};
		virtual void Bind() {};

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		BufferInfo GetBufferInfo() { return m_info; }

		void SetBufferInfo(BufferInfo buffer_info) { m_info = buffer_info; }

	public:
	private:
		BufferInfo m_info;
		GHandle m_renderHandle;

	protected:
		// TODO: suggesting maybe the gpu buffer info should only debug build ==> "BufferInfo m_info"


	};


}
