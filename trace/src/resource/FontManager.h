#pragma once

#include "core/Core.h"
#include "core/Enums.h"

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
		bool LoadFont(const std::string& name);
		Font* GetFont(const std::string& name);
		std::string GetFontResourcePath() { return font_resource_path.string(); }
		void UnloadFont(Font* font);

		static FontManager* get_instance();

	private:
		std::vector<Font> m_fonts;
		HashTable<uint32_t> hashTable;
		uint32_t m_numEntries;
		std::filesystem::path font_resource_path;

		static FontManager* s_instance;

	protected:

	};

}
