#pragma once

#include "resource/Ref.h"
#include "render/Material.h"
#include "FileStream.h"
#include "AssetsInfo.h"
#include "scene/UUID.h"
#include "MemoryStream.h"

#include <string>


namespace trace {

	class MaterialSerializer
	{

	public:

		static bool Serialize(Ref<MaterialInstance> material, const std::string& file_path);
		static bool Serialize(Ref<MaterialInstance> material, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		static Ref<MaterialInstance> Deserialize(const std::string& file_path);
		static bool Deserialize(Ref<GPipeline> pipeline, MaterialInstance* material, MemoryStream& stream);

	private:

	protected:

	};
}
