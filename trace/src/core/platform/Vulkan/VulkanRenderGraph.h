#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "VKtypes.h"
#include "VkUtils.h"
#include "render/GDevice.h"
#include "render/render_graph/RenderGraph.h"

namespace vk {


	bool __BuildRenderGraph(trace::GDevice* device, trace::RenderGraph* render_graph);
	bool __DestroyRenderGraph(trace::GDevice* device, trace::RenderGraph* render_graph);
	bool __BeginRenderGraphPass(trace::RenderGraph* render_graph, trace::RenderGraphPass* pass);
	bool __EndRenderGraphPass(trace::RenderGraph* render_graph, trace::RenderGraphPass* pass);
	bool __BeginRenderGraph(trace::RenderGraph* render_graph);
	bool __EndRenderGraph(trace::RenderGraph* render_graph);
}
