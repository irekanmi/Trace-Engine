#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "core/FileSystem.h"
#include "GHandle.h"
#include "resource/Resource.h"

namespace trace {

 	class TRACE_API GShader : public Resource
	{
	public:
		GShader();
		virtual ~GShader();

		GHandle* GetRenderHandle() { return &m_renderHandle; }
		ShaderStage GetShaderStage() { return m_stage; }
		void SetShaderStage(ShaderStage stage) { m_stage = stage; }
		std::vector<uint32_t>& GetCode() { return m_code; }
		void SetCode(const std::vector<uint32_t>& code) { m_code = code; }

	private:
		ShaderStage m_stage = ShaderStage::STAGE_NONE;
		GHandle m_renderHandle;
		std::vector<uint32_t> m_code;
	protected:
	};

}


