#pragma once

#include "render/GBuffer.h"

namespace trace {

	class TRACE_API OpenGLBuffer : public GBuffer
	{

	public:
		OpenGLBuffer();
		OpenGLBuffer(void* data, size_t size);
		OpenGLBuffer(BufferInfo info);
		~OpenGLBuffer();

		virtual void* GetNativeHandle() override;
		virtual void SetData(void* data, size_t size) override;
		virtual void Bind() override;

	private:
		uint32_t m_handle;
		
	protected:

	};



}