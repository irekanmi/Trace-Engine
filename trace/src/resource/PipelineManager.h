#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "HashTable.h"
#include "Ref.h"
#include <stdint.h>
#include "render/Graphics.h"
#include "render/GPipeline.h"

namespace trace {


	class TRACE_API PipelineManager
	{

	public:
		PipelineManager();
		PipelineManager(uint32_t max_entires);
		~PipelineManager();

		bool Init(uint32_t max_entries);
		void ShutDown();

		bool CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill = true);
		GPipeline* GetPipeline(const std::string& name);
		Ref<GPipeline> GetDefault(const std::string& name);
		void Unload(GPipeline* pipeline);

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
		Ref<GPipeline> standard_pipeline;
		Ref<GPipeline> skybox_pipeline;

	protected:

	};

}
