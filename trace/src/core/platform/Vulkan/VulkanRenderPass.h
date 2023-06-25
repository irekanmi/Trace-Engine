#pragma once

#include "render/GRenderPass.h"
#include "VKtypes.h"



namespace vk {

	bool __CreateRenderPass(trace::GRenderPass* render_pass, trace::RenderPassDescription desc);
	bool __DestroyRenderPass(trace::GRenderPass* render_pass);

}