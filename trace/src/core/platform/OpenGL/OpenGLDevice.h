#pragma once

#include "render/GDevice.h"

namespace trace {

	class OpenGLDevice : public GDevice
	{

	public:
		OpenGLDevice();
		~OpenGLDevice();

		virtual bool Init() override;
		virtual void DrawElements(GBuffer* vertex_buffer) override;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) override;
		virtual void DrawIndexed(GBuffer* index_buffer) override;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) override;
		virtual void ShutDown() override;

		virtual bool BeginFrame() override;
		virtual void EndFrame() override;

	private:
	protected:

	};

}
