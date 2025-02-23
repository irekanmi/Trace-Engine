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
		static bool Serialize(Ref<MaterialInstance> material, DataStream* stream);
		static Ref<MaterialInstance> Deserialize(const std::string& file_path);
		static Ref<MaterialInstance> Deserialize(DataStream* stream);
		static bool Deserialize(Ref<GPipeline> pipeline, MaterialInstance* material, MemoryStream& stream);

	private:

	protected:

	};
}
