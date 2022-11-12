#pragma once

#include <Core.h>
#include "GPUtypes.h"

namespace trace {

	class TRACE_API GContext
	{

	public:
		GContext();
		virtual ~GContext();

		virtual void Init() = 0;

		static GContext* get_instance() { return s_instance; }
		static RenderAPI get_render_api() { return s_API; }
		static RenderAPI s_API;
		static GContext* s_instance;

	private:
	protected:


	};

}
