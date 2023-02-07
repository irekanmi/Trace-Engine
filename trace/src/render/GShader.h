#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "core/FileSystem.h"

namespace trace {

	class TRACE_API GShader
	{
	public:
		GShader();
		~GShader();

		static GShader* Create_(std::string& src);
		static GShader* Create_(FileHandle& file);

	public:
		ShaderStage m_stage = ShaderStage::NONE;

	private:
	protected:
	};

}
