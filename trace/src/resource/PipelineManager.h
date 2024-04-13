#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "HashTable.h"
#include "Ref.h"
#include "render/Graphics.h"
#include "render/GPipeline.h"
#include "scene/UUID.h"
#include "serialize/FileStream.h"
#include "serialize/AssetsInfo.h"


#include <stdint.h>

namespace trace {


	class TRACE_API PipelineManager
	{

	public:
		PipelineManager();
		PipelineManager(uint32_t max_entires);
		~PipelineManager();

		bool Init(uint32_t max_entries);
		void ShutDown();

		Ref<GPipeline> CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill = true);
		Ref<GPipeline> GetPipeline(const std::string& name);
		Ref<GPipeline> GetDefault(const std::string& name);
		bool RecreatePipeline(Ref<GPipeline> pipeline, PipelineStateDesc desc);
		void Unload(GPipeline* pipeline);
		bool BuildDefaultPipelines(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		bool BuildDefaultPipelineShaders(FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);

		bool LoadDefaults();

		static PipelineManager* get_instance();
	private:
		static PipelineManager* s_instance;
		void unloadDefault(GPipeline* pipeline);

	private:
		std::vector<GPipeline> m_pipelines;
		uint32_t m_numEntries;
		uint32_t m_pipelineTypeSize;
		HashTable<uint32_t> m_hashtable;
		Ref<GPipeline> skybox_pipeline;
		Ref<GPipeline> light_pipeline;
		Ref<GPipeline> gbuffer_pipeline;
		Ref<GPipeline> text_batch_pipeline;
		Ref<GPipeline> text_pipeline;
		Ref<GPipeline> quad_pipeline;
		Ref<GPipeline> debug_line_pipeline;

	protected:

	};

}
