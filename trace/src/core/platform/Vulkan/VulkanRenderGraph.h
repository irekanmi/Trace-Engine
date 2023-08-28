#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "VKtypes.h"
#include "VkUtils.h"
#include "render/GDevice.h"
#include "render/render_graph/RenderGraph.h"

namespace trace {
	class GPipeline;
}

namespace vk {


	bool __BuildRenderGraph(trace::GDevice* device, trace::RenderGraph* render_graph);
	bool __DestroyRenderGraph(trace::GDevice* device, trace::RenderGraph* render_graph);
	bool __BeginRenderGraphPass(trace::RenderGraph* render_graph, trace::RenderGraphPass* pass);
	bool __EndRenderGraphPass(trace::RenderGraph* render_graph, trace::RenderGraphPass* pass);
	bool __BeginRenderGraph(trace::RenderGraph* render_graph);
	bool __EndRenderGraph(trace::RenderGraph* render_graph);
	bool __BindRenderGraphTexture(trace::RenderGraph* render_graph, trace::GPipeline* pipeline, const std::string& bind_name, trace::ShaderResourceStage resource_stage, trace::RenderGraphResource* resource, uint32_t index = 0);
	bool __BindRenderGraphBuffer(trace::RenderGraph* render_graph, trace::GPipeline* pipeline, const std::string& bind_name, trace::ShaderResourceStage resource_stage, trace::RenderGraphResource* resource, uint32_t index = 0);
}
