#include "pch.h"
#include "OpenGLDevice.h"
#include "GL/glew.h"

namespace trace {
	OpenGLDevice::OpenGLDevice()
	{
	}
	OpenGLDevice::~OpenGLDevice()
	{
	}
	void OpenGLDevice::Init()
	{
	}
	void OpenGLDevice::DrawElements(GBuffer* vertex_buffer)
	{
		vertex_buffer->Bind();
		glDrawArrays(GL_TRIANGLES, 0, vertex_buffer->GetCount());
	}
	void OpenGLDevice::DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances)
	{
		vertex_buffer->Bind();
		glDrawArraysInstanced(GL_TRIANGLES, 0, vertex_buffer->GetCount(), instances);
	}
	void OpenGLDevice::DrawIndexed(GBuffer* index_buffer)
	{
		index_buffer->Bind();
		glDrawElements(GL_TRIANGLES, index_buffer->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLDevice::DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances)
	{
		index_buffer->Bind();
		glDrawElementsInstanced(GL_TRIANGLES, index_buffer->GetCount(), GL_UNSIGNED_INT, nullptr, instances);
	}
	void OpenGLDevice::ShutDown()
	{
	}
	bool OpenGLDevice::BeginFrame()
	{
		return true;
	}
	void OpenGLDevice::EndFrame()
	{
	}
}