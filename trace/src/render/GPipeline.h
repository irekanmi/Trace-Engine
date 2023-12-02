#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "resource/HashTable.h"
#include "resource/Resource.h"
#include "GHandle.h"

namespace trace {

	class GTexture;

	class TRACE_API GPipeline : public Resource
	{

	public:
		GPipeline();
		~GPipeline();

		PipelineStateDesc& GetDesc() { return m_desc; }

		bool Initialize() { return false; };

		void SetData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t slot = 0, uint32_t index = 0) {};
		void SetTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t slot = 1, uint32_t index = 0) {};

		void SetData(const std::string& resource_name ,ShaderResourceStage resource_scope, void* data, uint32_t size) {};
		void SetTextureData(const std::string& resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index = 0) {};

		void SetMultipleData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t count, uint32_t slot = 0, uint32_t index = 0) {};
		void SetMultipleTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t count, uint32_t slot = 1, uint32_t index = 0) {};


		GHandle* GetRenderHandle() { return &m_renderHandle; }
		HashTable<uint32_t> _hashTable;
		std::vector<UniformMetaData> Scene_uniforms = {};
		std::vector<std::pair<uint32_t, uint32_t>> Scence_struct = {};
		PipelineStateDesc m_desc;
		uint32_t pipeline_type = 0;
	private:
		GHandle m_renderHandle;

	protected:


	};

}

