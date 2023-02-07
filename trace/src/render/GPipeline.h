#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"

namespace trace {

	class TRACE_API GPipeline
	{

	public:
		GPipeline();
		~GPipeline();

		PipelineStateDesc& GetDesc() { return m_desc; }

		static GPipeline* Create_(PipelineStateDesc desc);


	private:
	protected:
		PipelineStateDesc m_desc;

	};

}
