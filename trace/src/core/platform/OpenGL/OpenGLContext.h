#pragma once

#include <Core.h>
#include <Coretypes.h>
#include <Enums.h>

#include "render/GPUtypes.h"
#include "render/GContext.h"

namespace trace {

	class TRACE_API OpenGLContext : public GContext
	{

	public:
		OpenGLContext();
		~OpenGLContext();

		virtual void Init() override;
		virtual void DrawElements(GBuffer* vertex_buffer) override;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) override;
		virtual void DrawIndexed(GBuffer* index_buffer) override;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) override;

	private:
	protected:

	};

}
