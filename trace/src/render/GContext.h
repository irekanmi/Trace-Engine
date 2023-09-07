#pragma once

#include "core/Core.h"
#include "Graphics.h"
#include "GHandle.h"

namespace trace {

	class TRACE_API GContext
	{

	public:
		GContext();
		~GContext();

		void Update(float deltaTime);

		void Init();
		void ShutDown();

		GHandle* GetRenderHandle() { return &m_renderHandle; }

	private:
		GHandle m_renderHandle;
	protected:


	};

}
