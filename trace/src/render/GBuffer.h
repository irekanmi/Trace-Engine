#pragma once

#include "core/Core.h"
#include "Graphics.h"


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

		static GBuffer* Create_(const BufferInfo& buffer_info);
		static void Create_(const BufferInfo& buffer_info, GBuffer* dst);
	private:
	protected:
		// TODO: suggesting maybe the gpu buffer info should only debug build ==> "BufferInfo m_info"
		BufferInfo m_info;


	};


}
