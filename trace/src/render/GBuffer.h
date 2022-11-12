#pragma once

#include <Core.h>
#include "GPUtypes.h"


namespace trace {


	class TRACE_API GBuffer
	{

	public:
		GBuffer();
		virtual ~GBuffer();

		virtual void* GetNativeHandle();

		static GBuffer* Create_(const BufferInfo& buffer_info);
		static void Create_(const BufferInfo& buffer_info, GBuffer* dst);
		static GBuffer Create(const BufferInfo& buffer_info);
	private:
	protected:
		// TODO: suggesting maybe the gpu buffer info should only debug build ==> "BufferInfo m_info"
		BufferInfo m_info;


	};


	class TRACE_API VertexBuffer
	{

	public:
		VertexBuffer();
		virtual ~VertexBuffer();

		virtual void* GetNativeHandle();

		virtual void Bind();

		static VertexBuffer* Create_(size_t stride, size_t count);
		static void Create_(VertexBuffer* dst);
		static VertexBuffer Create(size_t stride, size_t count);
		static VertexBuffer* Create_(Vertex* vertices, size_t count);
	private:
	protected:
		GBuffer* m_buffer;

	};

}
