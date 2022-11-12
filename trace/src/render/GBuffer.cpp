#include <pch.h>

#include "GBuffer.h"

namespace trace {

	GBuffer::GBuffer()
	{

	}

	GBuffer::~GBuffer()
	{

	}

	GBuffer* GBuffer::Create_(const BufferInfo& buffer_info)
	{
		return nullptr;
	}

	void GBuffer::Create_(const BufferInfo& buffer_info, GBuffer* dst)
	{
		return;
	}
	GBuffer GBuffer::Create(const BufferInfo& buffer_info)
	{
		return GBuffer();
	}

	void* GBuffer::GetNativeHandle()
	{
		return nullptr;
	}

	VertexBuffer::VertexBuffer()
		: m_buffer(nullptr)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void* VertexBuffer::GetNativeHandle()
	{
		return nullptr;
	}

	void VertexBuffer::Bind()
	{
	}

	VertexBuffer* VertexBuffer::Create_(size_t stride, size_t count)
	{
		return nullptr;
	}

	void VertexBuffer::Create_(VertexBuffer* dst)
	{
	}

	VertexBuffer VertexBuffer::Create(size_t stride, size_t count)
	{
		return VertexBuffer();
	}

	VertexBuffer* VertexBuffer::Create_(Vertex* vertices, size_t count)
	{
		return nullptr;
	}

}