#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "render/Material.h"
#include "VKtypes.h"


namespace vk {

	bool __InitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline, trace::Material material);
	bool __PostInitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline, trace::Material material);
	bool __ApplyMaterial(trace::MaterialInstance* mat_instance);

}
