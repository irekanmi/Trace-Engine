#pragma once

#include <Core.h>
#include "GPUtypes.h"
#include "GBuffer.h"

namespace trace {

	class TRACE_API GContext
	{

	public:
		GContext();
		virtual ~GContext();

		virtual void Init() = 0;
		virtual void DrawElements(GBuffer* vertex_buffer) = 0;
		virtual void DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances) = 0;
		virtual void DrawIndexed(GBuffer* index_buffer) = 0;
		virtual void DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances) = 0;

		static RenderAPI get_render_api() { return s_API; }
		static RenderAPI s_API;

	private:
	protected:


	};

}
