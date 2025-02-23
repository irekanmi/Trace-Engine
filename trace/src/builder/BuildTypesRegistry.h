#pragma once

#include "reflection/TypeRegistry.h"
#include "scene/UUID.h"
#include "serialize/AssetsInfo.h"

#include <unordered_map>

namespace trace {

	REGISTER_KEY_VALUE_CONTAINER(std::unordered_map, UUID, AssetHeader);

}
