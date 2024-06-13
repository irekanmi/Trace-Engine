#pragma once

#include "render/GPipeline.h"
#include "VKtypes.h"



namespace vk {

	bool __CreatePipeline(trace::GPipeline* pipeline, trace::PipelineStateDesc desc);
	bool __DestroyPipeline(trace::GPipeline* pipeline);
	bool __InitializePipeline(trace::GPipeline* pipeline);
	bool __ShutDownPipeline(trace::GPipeline* pipeline);
	bool __SetPipelineData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, void* data, uint32_t size);
	bool __SetPipelineInstanceData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, void* data, uint32_t size,uint32_t count);
	bool __SetPipelineData_Meta(trace::GPipeline* pipeline, trace::UniformMetaData& meta_data, trace::ShaderResourceStage resource_scope, void* data, uint32_t size);
	bool __SetPipelineTextureData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, trace::GTexture* texture, uint32_t index = 0);
	bool __SetPipelineTextureData_Meta(trace::GPipeline* pipeline, trace::UniformMetaData& meta_data, trace::ShaderResourceStage resource_scope, trace::VKImage* texture, uint32_t index = 0);
	bool __BindPipeline_(trace::GPipeline* pipeline);

}
