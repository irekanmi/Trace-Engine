#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "serialize/AssetsInfo.h"
#include "scene/UUID.h"

#include "render/Font.h"
#include "HashTable.h"
#include <filesystem>

namespace trace {

	class TRACE_API FontManager
	{

	public:

		bool Init(uint32_t num_font_units);
		void Shutdown();

		bool LoadDefaults();
		Ref<Font> LoadFont(const std::string& name);
		Ref<Font> LoadFont_(const std::string& path);
		Ref<Font> GetFont(const std::string& name);
		std::string GetFontResourcePath() { return font_resource_path.string(); }
		void UnloadFont(Font* font);
		void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}
		Ref<Font> LoadFont_Runtime(UUID id);

		static FontManager* get_instance();

	private:
		std::vector<Font> m_fonts;
		HashTable<uint32_t> hashTable;
		uint32_t m_numEntries;
		std::filesystem::path font_resource_path;
		std::unordered_map<UUID, AssetHeader> m_assetMap;


	protected:

	};

}
