#pragma once

#include <Core.h>
#include "Graphics.h"
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


	private:
	protected:


	};

}
