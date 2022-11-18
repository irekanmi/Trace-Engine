#include <pch.h>

#include "OpenGLContext.h"
#include "GL/glew.h"

namespace trace {



	OpenGLContext::OpenGLContext()
	{
	}

	OpenGLContext::~OpenGLContext()
	{
	}

	void OpenGLContext::Init()
	{
	}

	void OpenGLContext::DrawElements(GBuffer* vertex_buffer)
	{
		vertex_buffer->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertex_buffer->GetCount());
	}

	void OpenGLContext::DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances)
	{
		vertex_buffer->Bind();
		glDrawArraysInstanced(GL_TRIANGLES, 0, vertex_buffer->GetCount(), instances);
	}

	void OpenGLContext::DrawIndexed(GBuffer* index_buffer)
	{
		index_buffer->Bind();
		glDrawElements(GL_TRIANGLES, index_buffer->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLContext::DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances)
	{
		index_buffer->Bind();
		glDrawElementsInstanced(GL_TRIANGLES, index_buffer->GetCount(), GL_UNSIGNED_INT, nullptr, instances);
	}

}