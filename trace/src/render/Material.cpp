#include "pch.h"

#include "render/GPipeline.h"
#include "Material.h"
#include "core/Coretypes.h"
#include "external_utils.h"
#include "serialize/MaterialSerializer.h"
#include "resource/MaterialManager.h"

namespace trace {
	MaterialInstance::MaterialInstance()
	{
	}
	MaterialInstance::~MaterialInstance()
	{
	}
	Ref<MaterialInstance> MaterialInstance::Deserialize(UUID id)
	{
		Ref<MaterialInstance> result;

		if (AppSettings::is_editor)
		{

			std::filesystem::path p = GetPathFromUUID(id);
			result = MaterialSerializer::Deserialize(p.string());
		}
		else
		{
			result = MaterialManager::get_instance()->LoadMaterial_Runtime(id);
		}

		return result;
	}
}