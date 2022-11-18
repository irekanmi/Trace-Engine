#include <pch.h>

#include "OpenGLBuffer.h"
#include "GL/glew.h"

namespace trace {

	OpenGLBuffer::OpenGLBuffer()
		: m_handle(0)
	{

	}

	OpenGLBuffer::OpenGLBuffer(void* data, size_t size)
		: m_handle(0)
	{
		
	}

	OpenGLBuffer::OpenGLBuffer(BufferInfo info)
	{
		m_info = info;
		glGenBuffers(1, &m_handle);
		GLenum buffer_usage = 0;
		if (info.m_usage & BufferUsage::VERTEX_BUFFER)
		{
			buffer_usage |= GL_ARRAY_BUFFER;
		}
		if (info.m_usage & BufferUsage::INDEX_BUFFER)
		{
			buffer_usage |= GL_ELEMENT_ARRAY_BUFFER;
		}
		glBindBuffer(buffer_usage, m_handle);

		glBufferData(buffer_usage, info.m_size, info.m_data, GL_STATIC_DRAW);
		

	}

	OpenGLBuffer::~OpenGLBuffer()
	{
		glDeleteBuffers(1, &m_handle);
	}

	void* OpenGLBuffer::GetNativeHandle()
	{
		return &m_handle;
	}
	void OpenGLBuffer::SetData(void* data, size_t size)
	{
		Bind();
		GLenum buffer_usage = 0;
		if (m_info.m_usage & BufferUsage::VERTEX_BUFFER)
		{
			buffer_usage |= GL_ARRAY_BUFFER;
		}
		if (m_info.m_usage & BufferUsage::INDEX_BUFFER)
		{
			buffer_usage |= GL_ELEMENT_ARRAY_BUFFER;
		}
		glBufferData(buffer_usage, size, data, GL_STATIC_DRAW);
	}
	void OpenGLBuffer::Bind()
	{
		GLenum buffer_usage = 0;
		if (m_info.m_usage & BufferUsage::VERTEX_BUFFER)
		{
			buffer_usage |= GL_ARRAY_BUFFER;
		}
		if (m_info.m_usage & BufferUsage::INDEX_BUFFER)
		{
			buffer_usage |= GL_ELEMENT_ARRAY_BUFFER;
		}
		glBindBuffer(buffer_usage, m_handle);
	}
}