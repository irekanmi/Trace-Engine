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
		virtual ~GPipeline();

		PipelineStateDesc& GetDesc() { return m_desc; }

		virtual bool Initialize() { return false; };

		virtual void SetData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t slot = 0, uint32_t index = 0) {};
		virtual void SetTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t slot = 1, uint32_t index = 0) {};

		virtual void SetData(const std::string& resource_name ,ShaderResourceStage resource_scope, void* data, uint32_t size) {};
		virtual void SetTextureData(const std::string& resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index = 0) {};

		virtual void SetMultipleData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t count, uint32_t slot = 0, uint32_t index = 0) {};
		virtual void SetMultipleTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t count, uint32_t slot = 1, uint32_t index = 0) {};

		virtual void Bind() {};

		virtual void Shutdown() {};

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		static GPipeline* Create_(PipelineStateDesc desc);

		HashTable<uint32_t> _hashTable;
		std::vector<UniformMetaData> Scene_uniforms = {};
		PipelineStateDesc m_desc;
	private:
		GHandle m_renderHandle;

	protected:


	};

}

