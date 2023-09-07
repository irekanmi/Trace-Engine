#pragma once

#include "render/GDevice.h"

namespace trace {

	class OpenGLDevice : public GDevice
	{

	public:
		OpenGLDevice();
		~OpenGLDevice();

		bool Init();
		void DrawElements(GBuffer* vertex_buffer);
		void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances);
		void DrawIndexed(GBuffer* index_buffer);
		void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances);
		void ShutDown();

		bool BeginFrame(GSwapchain* swapchain);
		void EndFrame();

	private:
	protected:

	};

}
