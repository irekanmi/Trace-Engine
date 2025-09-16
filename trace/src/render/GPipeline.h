#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "resource/HashTable.h"
#include "resource/Resource.h"
#include "resource/Ref.h"
#include "GHandle.h"
#include "serialize/DataStream.h"
#include "scene/UUID.h"

namespace trace {

	class GTexture;
	class ShaderGraph;

	enum class MaterialType
	{
		NONE,
		OPAQUE_LIT,
		OPAQUE_UNLIT,
		TRANSPARENT_LIT,
		TRANSPARENT_UNLIT,
		PARTICLE_LIT,
		PARTICLE_UNLIT
	};

	class GPipeline : public Resource
	{

	public:
		GPipeline();
		virtual ~GPipeline();

		bool Create(PipelineStateDesc desc, bool auto_fill = true);
		virtual void Destroy() override;
		bool RecreatePipeline(PipelineStateDesc desc);

		PipelineStateDesc& GetDesc() { return m_desc; }

		bool Initialize() { return false; };

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		HashTable<uint32_t>& GetHashTable() { return m_hashTable; }
		std::vector<UniformMetaData>& GetSceneUniforms() { return m_sceneUniforms; }
		std::vector<std::pair<uint32_t, uint32_t>>& GetSceneStructs() { return m_scenceStruct; }
		uint32_t GetPipelineType() { return m_pipelineType; }
		std::unordered_map<std::string, InternalMaterialData>& GetShaderGraphVariables() { return m_shaderGraphVariables; }


		void SetSceneUniforms(std::vector<UniformMetaData>& uniforms) { m_sceneUniforms = std::move(uniforms); }
		void SetSceneStructs(std::vector<std::pair<uint32_t, uint32_t>>& structs) { m_scenceStruct = std::move(structs); }
		void SetPipelineType(uint32_t pipeline_type) { m_pipelineType = pipeline_type; }
		void SetDesc(PipelineStateDesc& desc) { m_desc = desc; }

		MaterialType GetType() { return m_type; }
		void SetType(MaterialType type) { m_type = type; }
		ShaderGraph* GetShaderGraph() { return m_shaderGraph; }
		void SetShaderGraph(ShaderGraph* shader_graph) { m_shaderGraph = shader_graph; }

		static Ref<GPipeline> Deserialize(UUID id);
		static Ref<GPipeline> Deserialize(DataStream* stream);

	private:
		HashTable<uint32_t> m_hashTable;
		std::vector<UniformMetaData> m_sceneUniforms = {};
		std::vector<std::pair<uint32_t, uint32_t>> m_scenceStruct = {};// NOTE: first-> the location of the structure, second-> the current frame offset
		PipelineStateDesc m_desc;
		uint32_t m_pipelineType = 0;
		GHandle m_renderHandle;
		std::unordered_map<std::string, InternalMaterialData> m_shaderGraphVariables;
		MaterialType m_type = MaterialType::OPAQUE_LIT;
		ShaderGraph* m_shaderGraph = nullptr;

	protected:


	};

}

