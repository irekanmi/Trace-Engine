#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Material.h"
#include "VKtypes.h"


namespace vk {

	bool __InitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline);
	bool __DestroyMaterial(trace::MaterialInstance* mat_instance);
	bool __PostInitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline);
	bool __ApplyMaterial(trace::MaterialInstance* mat_instance, int32_t render_graph_index = 0);

}
