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

		virtual void Update(float deltaTime) = 0;

		virtual void Init() = 0;
		virtual void ShutDown() = 0;

		static RenderAPI get_render_api() { return s_API; }
		static RenderAPI s_API;

	private:
	protected:


	};

}
