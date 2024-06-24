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

		HashTable<uint32_t>& GetHashTable() { return m_hashTable; }
		std::vector<UniformMetaData>& GetSceneUniforms() { return m_sceneUniforms; }
		std::vector<std::pair<uint32_t, uint32_t>>& GetSceneStructs() { return m_scenceStruct; }
		uint32_t GetPipelineType() { return m_pipelineType; }


		void SetSceneUniforms(std::vector<UniformMetaData>& uniforms) { m_sceneUniforms = std::move(uniforms); }
		void SetSceneStructs(std::vector<std::pair<uint32_t, uint32_t>>& structs) { m_scenceStruct = std::move(structs); }
		void SetPipelineType(uint32_t pipeline_type) { m_pipelineType = pipeline_type; }
		void SetDesc(PipelineStateDesc& desc) { m_desc = desc; }

	private:
		HashTable<uint32_t> m_hashTable;
		std::vector<UniformMetaData> m_sceneUniforms = {};
		std::vector<std::pair<uint32_t, uint32_t>> m_scenceStruct = {};// NOTE: first-> the location of the structure, second-> the current frame offset
		PipelineStateDesc m_desc;
		uint32_t m_pipelineType = 0;
		GHandle m_renderHandle;

	protected:


	};

}

