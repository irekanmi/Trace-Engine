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

	private:
	protected:

	};

}
