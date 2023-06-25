#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "core/FileSystem.h"
#include "GHandle.h"

namespace trace {

 	class TRACE_API GShader
	{
	public:
		GShader();
		virtual ~GShader();

		GHandle* GetRenderHandle() { return &m_renderHandle; }
		ShaderStage GetShaderStage() { return m_stage; }
		void SetShaderStage(ShaderStage stage) { m_stage = stage; }

	private:
		ShaderStage m_stage = ShaderStage::STAGE_NONE;
		GHandle m_renderHandle;
	protected:
	};

}


