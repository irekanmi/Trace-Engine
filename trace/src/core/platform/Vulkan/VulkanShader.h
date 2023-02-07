#pragma once

#include "render/GShader.h"
#include  "VKtypes.h"

namespace trace {

	class TRACE_API VulkanShader : public GShader
	{

	public:
		VulkanShader();
		~VulkanShader();

		
		VKShader m_handle;
	private:

	protected:

	};

}
