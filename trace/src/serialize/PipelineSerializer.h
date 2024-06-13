#pragma once

#include "resource/Ref.h"
#include "render/GPipeline.h"
#include "FileStream.h"
#include "MemoryStream.h"
#include "AssetsInfo.h"
#include "scene/UUID.h"

#include <string>


namespace trace {

	class PipelineSerializer
	{

	public:

		static bool Serialize(Ref<GPipeline> pipeline, const std::string& file_path);
		static bool Serialize(Ref<GPipeline> pipeline, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		static bool SerializeShader(Ref<GShader> shader, FileStream& stream, std::vector<std::pair<UUID, AssetHeader>>& map);
		static bool SerializeShader(GShader* shader, const std::string& file_path);
		static bool DeserializeShader(std::string& file_path, std::vector<uint32_t>& out_code, std::vector<std::pair<std::string, int>>& out_data_index);
		static Ref<GPipeline> Deserialize(const std::string& file_path);

	private:

	protected:

	};
}
