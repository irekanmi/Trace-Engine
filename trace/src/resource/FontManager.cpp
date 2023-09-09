#include "pch.h"

#include "FontManager.h"
#include "backends/Fontutils.h"
#include "render/GTexture.h"

namespace trace {

	FontManager* FontManager::s_instance = nullptr;

	bool FontManager::Init(uint32_t num_font_units)
	{
		m_numEntries = num_font_units;

		m_fonts.resize(m_numEntries);
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_fonts[i].m_id = INVALID_ID;
		}

		hashTable.Init(m_numEntries);
		hashTable.Fill(INVALID_ID);

		// Temp
		FontFuncLoader::Load_MSDF_Func();

		// Trying to determine directory where textures are located..............
		std::filesystem::path current_search_path("./assets");

		if (std::filesystem::exists(current_search_path))
		{
			current_search_path /= "fonts";
			if (std::filesystem::exists(current_search_path))
			{
				font_resource_path = current_search_path;
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "fonts";
				if (std::filesystem::exists(current_search_path))
				{
					font_resource_path = current_search_path;
				}
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "fonts";
				if (std::filesystem::exists(current_search_path))
				{
					font_resource_path = current_search_path;
				}
			}
		}
		else
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "fonts";
				if (std::filesystem::exists(current_search_path))
				{
					font_resource_path = current_search_path;
				}
			}
		}
		// .............................


		return true;
	}

	void FontManager::Shutdown()
	{
		if (!m_fonts.empty())
		{
			for (Font& font : m_fonts)
			{
				if (font.m_id == INVALID_ID)
					continue;
				TRC_DEBUG("Unloaded Font name:{}", font.GetFontName());
				FontFunc::DestroyFont(&font);
			}
			m_fonts.clear();
		}

	}

	bool FontManager::LoadDefaults()
	{
		return true;
	}

	bool FontManager::LoadFont(const std::string& name)
	{
		uint32_t hash = hashTable.Get(name);
		if (hash != INVALID_ID)
		{
			TRC_WARN("{} font has been loaded", name);
			return true;
		}

		uint32_t i = 0;
		for (Font& font : m_fonts)
		{
			if (font.m_id == INVALID_ID)
			{
				std::string file_path = (font_resource_path / name).string();
				font.SetFontName(name);
				if (!FontFunc::LoadAndInitializeFont(file_path, &font))
				{
					TRC_ERROR("Failed to load to font, path->{}", file_path);
					return false;
				}
				font.m_id = i;
				hashTable.Set(name, i);
				return true;
			}

			i++;
		}

		return true;
	}

	Font* FontManager::GetFont(const std::string& name)
	{
		uint32_t hash = hashTable.Get(name);
		if (hash == INVALID_ID)
		{
			TRC_WARN("{} font has not been loaded", name);
			return nullptr;
		}

		return &m_fonts[hash];
	}

	void FontManager::UnloadFont(Font* font)
	{
		// TODO: Destroy unreferenced font
	}

	FontManager* FontManager::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new FontManager;
		}
		return s_instance;
	}

}