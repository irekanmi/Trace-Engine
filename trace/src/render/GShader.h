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

		static GShader* Create_(std::string& src, ShaderStage stage);
		static GShader* Create_(FileHandle& file);

	public:
		ShaderStage m_stage = ShaderStage::STAGE_NONE;

	private:
		GHandle m_renderHandle;
	protected:
	};

}


