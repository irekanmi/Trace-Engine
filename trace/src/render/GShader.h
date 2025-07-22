#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "core/FileSystem.h"
#include "GHandle.h"
#include "resource/Resource.h"
#include "resource/Ref.h"
#include "scene/UUID.h"
#include "serialize/DataStream.h"

namespace trace {

 	class GShader : public Resource
	{
	public:
		GShader();
		virtual ~GShader();

		bool Create(const std::string& path, ShaderStage shader_stage);
		bool Create(std::vector<uint32_t>& code, std::vector<std::pair<std::string, int>>& data_index, ShaderStage shader_stage);
		virtual void Destroy() override;

		GHandle* GetRenderHandle() { return &m_renderHandle; }
		ShaderStage GetShaderStage() { return m_stage; }
		void SetShaderStage(ShaderStage stage) { m_stage = stage; }
		std::vector<uint32_t>& GetCode() { return m_code; }
		void SetCode(const std::vector<uint32_t>& code) { m_code = code; }
		std::vector<std::pair<std::string, int>>& GetDataIndex() { return m_instanceDataIndex; }

		static Ref<GShader> Deserialize(UUID id);
		static Ref<GShader> Deserialize(DataStream* stream);

	private:
		ShaderStage m_stage = ShaderStage::STAGE_NONE;
		GHandle m_renderHandle;
		std::vector<uint32_t> m_code;
		std::vector<std::pair<std::string, int>> m_instanceDataIndex;
	protected:
	};

}


